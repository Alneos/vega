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
      sh "docker save ${image} | tar -xf -"
      sh "docker push ${image}"
    }
    container('maven') {
      sh "mvn --version"
      sh "mvn deploy:deploy-file -DgroupId=fr.systemx.top -DartifactId=vegapp -Dversion=1.0.0-SNAPSHOT -DgeneratePom=false -Dpackaging=pom -Dfile=/srv/data/vegapp"
    }
  }
}
