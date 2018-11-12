#!/usr/bin/groovy
node('maven') {
  stage('Build Docker image') {
    def image = "harbor.irtsysx.fr/top/vegairt:1.0"
    checkout([$class: 'GitSCM', branches: [[name: '*/$GIT_BRANCH']], doGenerateSubmoduleConfigurations: false, extensions: [], submoduleCfg: [], userRemoteConfigs: [[credentialsId: PULL_KEY, url: GIT_URL]]])
    sh "pwd"
    sh "ls"
    sh "whoami"
    container('docker') {
      sh "echo hello from maven"
      sh "pwd"
      sh "ls"
      sh "whoami"
      sh "docker --version"
      sh "docker build -t ${image} -f Dockerfile_jenkins ."
      sh "docker save ${image} -o vegapp.tar"
      sh "docker images -q"
      sh "ls -l"
      sh "tar -xf vegapp.tar --strip-components 1 --to-stdout `tar -tf vegapp.tar | grep layer.tar` | gzip > vegapp.tgz"
      sh "ls -l"
      sh "docker push ${image}"
    }
    container('maven') {
      sh "mvn --version"
      sh "mvn deploy:deploy-file -Dclassifier=linux -DgroupId=fr.systemx.top -DartifactId=vegapp -Dversion=1.0.0-SNAPSHOT -DgeneratePom=false -Dfile=vegapp.tgz -Dpackaging=tgz -Durl=http://nexus.factory:8081/repository/maven-snapshots/ -DrepositoryId=smite-nexus"
    }
  }
}
