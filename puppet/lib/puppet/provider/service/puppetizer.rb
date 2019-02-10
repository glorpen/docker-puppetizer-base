# Daemontools service management
#
# author Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>

#
# TODO: puppetizer as default in container.. facter[pouppetizer] = true?
#
Puppet::Type.type(:service).provide :puppetizer, :parent => :runit do
  desc <<-'EOT'
    Runit service management contained in puppetizer env.

    This provider manages daemons running supervised by Runit.
    When detecting the service directory it will check, in order of preference:

    * `/opt/puppetizer/service`

    The daemon directory should be in one of the following locations:

    * `/opt/puppetizer/etc/service`

    This provider supports out of the box:

    * start/stop
    * enable/disable
    * restart
    * status


  EOT

  commands :sv => '/opt/puppetizer/bin/sv'

  class << self
    def defpath
      "/opt/puppetizer/etc/service"
    end
  end

  def default
    true
  end

  def sv(*args)
    execute([command(:sv)] + args)
  end

  def servicedir
    "/opt/puppetizer/service"
  end

  def start
    if enabled? != :true
        enable
        # Work around issue #4480
        # runsvdir takes up to 5 seconds to recognize
        # the symlink created by this call to enable
        #TRANSLATORS 'runsvdir' is a linux service name and should not be translated
        Puppet.info _("Waiting up to 5 seconds for runsvdir to discover service %{service}") % { service: self.service }
        5.times do
          sleep 1
          begin
            z = sv "status", self.service
            break
          rescue Puppet::ExecutionFailure => detail
            unless detail.message =~ /fail: .*runsv not running$/
              raise Puppet::Error.new( "Could not enable service #{resource.ref}: #{detail}", detail )
            end
          end
        end
    end
    sv "start", self.service
  end
  
end
