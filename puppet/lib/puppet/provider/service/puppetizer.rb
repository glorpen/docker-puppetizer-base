# Puppetizer/runit service management
#
# author Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>

Puppet::Type.type(:service).provide :puppetizer, :parent => :runit do
  desc <<-'EOT'
    Runit service management contained in puppetizer env.

    This provider manages daemons running supervised by Runit.
    When detecting the service directory it will check, in order of preference:

    * `/opt/puppetizer/service`

    The daemon directory should be in the following location:

    * `/opt/puppetizer/etc/service`

    This provider supports out of the box:

    * start/stop
    * enable/disable
    * restart
    * status

  EOT

  commands :sv => '/opt/puppetizer/bin/sv'

  def servicedefdir
    "/opt/puppetizer/etc/service"
  end

  class << self
    def defpath
      "/opt/puppetizer/etc/service"
    end
  end

  def default
    true
  end

  def down_file
    File.join(servicedefdir, resource[:name], "down")
  end

  def sv(*args)
    execute([command(:sv)] + args)
  end

  def servicedir
    "/opt/puppetizer/service"
  end

  def toggle_down(present)
    unless Puppet::FileSystem.exist?(File.dirname(down_file))
      raise Puppet::Error.new( "Could not toggle service mode #{resource.ref}: Service directory #{down_file} does not exist" )
    end

    if present
      unless Puppet::FileSystem.exist?(down_file)
        Puppet::FileSystem.touch(down_file)
      end
    else
      if Puppet::FileSystem.exist?(down_file)
        Puppet::FileSystem.unlink(down_file)
      end
    end
  end

  def installed?
    Puppet::Type::Service::ProviderDaemontools.instance_method(:enabled?).bind(self).call
  end

  def install
    if installed? != :true
      Puppet::Type::Service::ProviderDaemontools.instance_method(:enable).bind(self).call

      if Facter.value('puppetizer')['running']
        # Work around issue #4480
        # runsvdir takes up to 5 seconds to recognize
        # the symlink created by this call to enable
        #TRANSLATORS 'runsvdir' is a linux service name and should not be translated
        Puppet.info _("Waiting up to 5 seconds for runsvdir to discover service %{service}") % { service: self.service }

        lastError = nil
        5.times do
          sleep 1
          begin
            sv "status", self.service
            lastError = nil
            break
          rescue Puppet::ExecutionFailure => detail
            lastError = detail
          end
        end

        if lastError
          raise Puppet::Error.new( "Could not install service #{resource.ref}: #{lastError}", lastError )
        end
      end
    end
  end

  def disable
    toggle_down(true)
    install
  end

  def enabled?
    return Puppet::FileSystem.exist?(down_file) ? :false : :true
  end

  def enable
    toggle_down(false)
    install
  end

  def start
    install
    if Facter.value('puppetizer')['running']
      sv "start", self.service
    end
  end

  def stop
    install
    if Facter.value('puppetizer')['running']
      super
    end
  end

  def restart
    install
    if Facter.value('puppetizer')['running']
      super
    end
  end
end
