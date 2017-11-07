define puppetizer::health(
  Optional[String] $shebang = '/bin/bash',
  String $content
){
  include ::puppetizer
  
  if $shebang == undef {
    $_content = $content
  } else {
    $_content = "#!${shebang}\n${content}"
  }
  
  file { "${::puppetizer::health_scripts_path}/${name}":
    content => $_content,
    mode => 'a=,u=rx',
    require => File[$::puppetizer::health_scripts_path]
  }
}
