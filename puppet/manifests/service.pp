define puppetizer::service (
  Optional[String] $run_content = undef,
  Optional[String] $run_source = undef,

  Optional[String] $finish_content = undef,
  Optional[String] $finish_source = undef,

  Boolean $autostart = false
){
  $_dir = "/opt/puppetizer/etc/service/${name}"

  file { $_dir:
    ensure => directory
  }

  $file_opts = {
    mode    => 'u=rwx,a=rx',
    backup  => false,
    notify  => Service[$title]
  }

  file { "${_dir}/run":
    content => $run_content,
    source  => $run_source,
    *       => $file_opts
  }

  if $finish_content or $finish_source {
    file { "${_dir}/finish":
      content => $finish_content,
      source  => $finish_source,
      *       => $file_opts
    }
  }

  $svc_opts = {
    provider   => 'puppetizer',
    enable     => $autostart,
    ensure     => $facts['puppetizer']['running'],
    hasstatus  => true,
    hasrestart => true
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
