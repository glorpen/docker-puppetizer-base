class puppetizer {
  $puppetizer_var = '/opt/puppetizer'
  $health_scripts_path = "${puppetizer_var}/health"

  # no need for backups inside container
  File <| |> {
    backup => false
  }
}
