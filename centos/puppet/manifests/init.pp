class puppetizer {
  
  #$running = $::facts['puppetizer']['running']
  #$building = $::facts['puppetizer']['building']
  
  $puppetizer_var = '/var/opt/puppetizer'
  $health_scripts_path = "${puppetizer_var}/health"
  $service_scripts_path = "${puppetizer_var}/services"
  
  file { [$health_scripts_path, $service_scripts_path]:
    ensure => directory,
    purge => true,
    backup => false
  }
  
  puppetizer_scripts { 'collect':
    service_scripts => $service_scripts_path,
    require => [Class['puppetizer::last'], File[$service_scripts_path]] 
  }
  
  # TODO: add to documentation: service scripts should be blocking
  # /etc/puppetizer.d/
  #   services/
  #     *.stop
  #     *.status
  #   scripts/
  #     *.init
  #     *.start
  #     *.stop
  #     *.shutdown
  #   health/* + puppetizer_health { 'asd': command => '#!/bin/bash\nasd', shebang=>'/bin/bash'}
}
