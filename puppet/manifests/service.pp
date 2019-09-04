define puppetizer::service (
  Optional[String] $start_content = undef,
  Optional[String] $start_source = undef,
  Optional[String] $stop_content = undef,
  Optional[String] $stop_source = undef,
){
  $_dir = "/opt/puppetizer/etc/services"
  $_start_script = "${_dir}/${name}.start"
  $_stop_script = "${_dir}/${name}.stop"

  $file_opts = {
    mode    => 'a=rx,u+w',
    backup  => false,
  }

  file { $_start_script:
    content => $start_content,
    source  => $start_source,
    notify  => Service[$title],
    *       => $file_opts
  }
  file { $_stop_script:
    content => $stop_content,
    source  => $stop_source,
    before  => Service[$title],
    *       => $file_opts
  }

  $svc_opts = {
    provider   => 'puppetizer',
    ensure     => $facts['puppetizer']['running'] and ! $facts['puppetizer']['halting'],
    hasstatus  => true,
    hasrestart => false
  }

  if defined(Service[$title]) {
    Service <| title == $title |> {
      * => $svc_opts
    }
  } else {
    service { $title:
      * => $svc_opts
    }
  }
}
