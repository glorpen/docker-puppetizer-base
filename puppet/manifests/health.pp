define puppetizer::health(
  Optional[String] $interpreter = undef,
  String $command
){
  include ::puppetizer
  
  if $interpreter == undef {
    $_content = $command
  } else {
    $_content = "#!${interpreter}\n${command}"
  }
  
  file { "${::puppetizer::health_scripts_path}/${name}":
    content => $_content,
    mode => 'a=,u=rx'
  }
}
