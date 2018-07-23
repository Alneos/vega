#!/usr/bin/groovy


podTemplate(label: 'dockerpod',
  containers: [containerTemplate(name: 'docker', image: 'docker:1.11', ttyEnabled: true, command: 'cat')],
  volumes: [hostPathVolume(hostPath: '/var/run/docker.sock', mountPath: '/var/run/docker.sock')]
  ) {

  def image = "gitlab/vegairt"
  node('dockerpod') {
    stage('Build Docker image') {
      checkout([$class: 'GitSCM', branches: [[name: '*/$GIT_BRANCH']], doGenerateSubmoduleConfigurations: false, extensions: [], submoduleCfg: [], userRemoteConfigs: [[credentialsId: PULL_KEY, url: GIT_URL]]])
      sh "echo hello from jenkins"
      sh "pwd"
      sh "ls"
      sh "whoami"
      container('docker') {
        sh "echo hello from docker"
        sh "pwd"
        sh "ls"
        sh "whoami"          
        sh "docker build -t ${image} -f Dockerfile_jenkins ."
      }
    }
  }
}
