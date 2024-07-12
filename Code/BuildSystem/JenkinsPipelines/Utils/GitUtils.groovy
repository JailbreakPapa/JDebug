def cloneRepo(REPO_BRANCH , REPO_URL ) {
  checkout scmGit(
    branches: [[name: REPO_BRANCH ]],
    extensions: [], 
    userRemoteConfigs: [
      [
        credentialsId: variables.BB_CREDENTIAL_ID,
        url: variables.REPO_URL
      ]
    ]
  )
}