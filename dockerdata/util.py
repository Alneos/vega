# -*- coding: utf-8 -*-

import os
import re
import time
import difflib
from stat import ST_MODE
from subprocess     import Popen, PIPE, STDOUT
from distutils      import log, file_util, dir_util

YES_VALUES = ['yes', 'oui', 'y', 'o']

import traceback
def tick(msg=''):
    ct =  time.time()
    msecs = (ct - long(ct)) * 1000
    stack = traceback.format_stack(limit=10)
    mat = re.search('File [\'\"]*(.*?)[\'\"]*, *line ([0-9]+), *in (.*)', stack[-2])
    orig = '[%s@%s:%s]' % (mat.group(3), mat.group(1), mat.group(2))
    log.debug("TIME : %s.%03d %s %s", time.strftime('%H:%M:%S'), msecs, msg, orig)

def execute(cmd, env=None):
   """Executes a command and returns its output."""
   p = Popen(cmd, stdout=PIPE, stderr=STDOUT, close_fds=True, env=env)
   return p.communicate()[0]


def less_than_version(vers1, vers2):
   return version2tuple(vers1) < version2tuple(vers2)


def version2tuple(vers_string):
   """1.7.9alpha --> (1, 7, 9, 'alpha'), 1.8 --> (1, 8, 0, 'final')"""
   tupl0 = vers_string.split('.')
   val = []
   for v in tupl0:
      m = re.search('(^[0-9]+)(.*)', v)
      if m:
         val.append(int(m.group(1)))
         if m.group(2):
            val.append(m.group(2).replace('-', '').replace('_', '').strip())
      else:
         val.append(v)
   val.extend([0]*(3-len(val)))
   if type(val[-1]) in (int, long):
      val.append('final')
   return tuple(val)


def get_function(functions, vers):
   """Return the function to be used starting from version...
   functions = (
      ('0.0.0', f1),
      ('1.7.5', f2),
      ('1.8.0', f3)
   )
   """
   tv, func = functions[0]
   for vi, fi in functions[1:]:
      if less_than_version(vers, vi):
         break
      func = fi
   return func


def prefix2etc(prefix):
   if prefix == "/usr":
      return "/etc"
   else:
      return os.path.join(prefix, 'etc')


def move_tree(src, dirs_and_files, dest, preserve_symlinks=0, dry_run=0,
              same_device=True):
   """Moves dirs_and_files from src to dest
   (put dirs first to create destination dirs for files).
   same_device=True allows to simply rename 'src_i' to 'dst_i', if not
   the files are copied and removed.
   """
   dir_util.create_tree(dest, dirs_and_files, dry_run=dry_run)
   for obj in dirs_and_files:
      src_i = os.path.join(src, obj)
      dst_i = os.path.join(dest, obj)
      log.debug("move_tree %s to %s" , src_i, dst_i)
      if not os.path.exists(src_i) and not os.path.islink(src_i):
         log.info("not found: %s", src_i)
         continue
      if os.path.islink(src_i):
         tick()
         link_dest = os.readlink(src_i)
         log.info("linking %s -> %s", dst_i, link_dest)
         if not dry_run:
            os.symlink(link_dest, dst_i)
            os.remove(src_i)
         tick()
      elif os.path.isdir(src_i):
         if same_device:
            tick('use os.rename for %s' % dst_i)
            #os.rename(src_i, dst_i)
            import shutil
            shutil.move(src_i, dst_i)
         else:
            tick('use copy/rm for %s' % dst_i)
            dir_util.copy_tree(src_i, dst_i, preserve_symlinks=preserve_symlinks, dry_run=dry_run)
            log.info('removing %s', src_i)
            remove_tree(src_i, dry_run=dry_run)
         tick()
      else:
         tick()
         file_util.move_file(src_i, dst_i, dry_run=dry_run)
         tick()


def remove_tree(directory, verbose=0, dry_run=0, empty_dirs=0):
   """Same as dir_util.remove_tree + try to remove parent directories if they are empty."""
   tick()
   dir_util.remove_tree(directory, verbose, dry_run)
   tick()
   if not empty_dirs:
      return
   dsplit = directory.split(os.sep)
   if dsplit[0] != '':
      return
   while len(dsplit) > 1:
      dsplit[0] = os.sep
      full = os.path.join(*dsplit)
      if os.path.exists(full):
         try:
            os.rmdir(full)
            log.debug("removed: %s", full)
         except:
            break
      del dsplit[-1]
   tick()


def remove_empty_dirs(fromdir):
   """Removes all empty dirs"""
   for base, dirs, files in os.walk(fromdir, topdown=False):
      try:
         os.rmdir(base)
      except:
         pass


def re_search(filename, regexp, flags=0):
   """Search for regexp in the content of 'filename'."""
   log.debug("search regular expression %r in %s", regexp, filename)
   if not os.path.isfile(filename):
      log.debug("not found: %s", filename)
      return None
   expr = re.compile(regexp, flags)
   content = open(filename, 'r').read()
   res = expr.search(content)
   return res


def check_and_store(dico, match, lvar):
   """Check match.groups() and store them in dico[*var]."""
   log.debug("check_var: variables=%r", lvar)
   if match is None:
      return
   if type(lvar) not in (tuple, list):
      lvar = [lvar,]
   inter = min(len(lvar), len(match.groups()))
   log.debug("           values=%r", match.groups())
   for i in range(inter):
      var, value = lvar[i], match.group(i+1).strip()
      if re.search('\?[A-Z_0-9]+\?', value):
         log.info("%s is not configured (%r)", var, value)
         continue
      dico[var] = value


# copy from asrun.utils
def read_rcfile(ficrc, destdict):
   """Read a ressource file and store variables to 'destdict'.
   """
   f = open(ficrc, 'r')
   for line in f:
      if not re.search('^[ ]*#', line):
         mat = re.search('([-a-z_A-Z0-9]+) *: *(.*)', line.strip())
         if mat:
            key   = mat.group(1)
            value = mat.group(2).strip()
            # try to convert numbers and boolean values
            try:
               if value.isdigit():
                  value = int(value)
               else:
                  value = float(value)
            except ValueError:
               if value in ('True', 'False'):
                  value = eval(value)
            # repeatable ? (space separated)
            if destdict.has_key(key) and key in ('vers', 'noeud'):
               value = str(destdict[key])+' '+str(value)
            destdict[key] = value
   f.close()


def replace_rcfile(ficrc, values):
   """Changes the values of fields of 'ficrc' using 'values' dict.
   """
   content = open(ficrc, 'r').read()
   for field, value in values.items():
      try:
         searched = '^%s *: *(.*)$' % field
         replacement = '%s : %s' % (field, value)
         # particular treatment for repeatable fields
         if field == 'noeud':    # only one occurrence in content, but may have more values
            replacement = os.linesep.join(['%s : %s' % (field, vali) for vali in value.split()])
         elif field == 'vers':
            searched = '^%s$' % re.escape('#?vers : VVV?')
            replacement = os.linesep.join(['#?vers : VVV?'] \
                           + ['%s : %s' % (field, vali) for vali in value.split()])
         regexp = re.compile(searched, re.MULTILINE)
         test = regexp.findall(content)
         if len(test) == 0:
             searched = '^#%s *: *(.*)$' % field
             regexp = re.compile(searched, re.MULTILINE)
             test = regexp.findall(content)
             if len(test) != 0:
                 log.warn("field %s is now optional, set to %s", field, value)
         content = regexp.sub(replacement, content)
      except: # skip invalid regexp (probably not a real field)
         pass
   open(ficrc, 'w').write(content)


def print_diff(fromfile, tofile):
   fromdate = time.ctime(os.stat(fromfile).st_mtime)
   todate = time.ctime(os.stat(tofile).st_mtime)
   fromlines = open(fromfile, 'U').readlines()
   tolines = open(tofile, 'U').readlines()

   diff = difflib.unified_diff(fromlines, tolines, fromfile, tofile,
                                    fromdate, todate)
   return ''.join(diff)


def backup_name(filename, ext, hidden=True):
   """From 'filename' + '.ext' returns '.ext-filename' if hidden is True, 'filename.ext' else."""
   if ext[0] != '.':
      ext = '.' + ext
   dirname, basename = os.path.split(filename)
   if hidden:
      basename = ext + '-' + basename
   else:
      basename = basename + ext
   return os.path.join(dirname, basename)


def chmod_scripts(files):
   if type(files) not in (list, tuple):
      files = [files,]
   for filename in files:
      if not os.access(filename, os.W_OK):
          log.warn("no sufficient permission to change '%s', skipped.", filename)
          continue
      mode = ((os.stat(filename)[ST_MODE]) | 0555) & 07777
      log.info("changing mode of %s to %o", filename, mode)
      os.chmod(filename, mode)


# unittest
def _test_get_function():
   ff = ( ('0.0.0', 'func < 1.7.5'), ('1.7.5', 'func >= 1.7.5'),
          ('1.8.0', 'func >= 1.8.0'), ('1.10.0', 'func >= 1.10'), )
   for v in ('1.2.3', '1.7.4', '1.7.5', '1.7.99', '1.8', '1.8.01',
            '1.10', '1.11', 'unknown',):
      print v, '-->', get_function(ff, v)
