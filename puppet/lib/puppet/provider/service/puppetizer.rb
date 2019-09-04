# Puppetizer init service management
#
# author Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>

Puppet::Type.type(:service).provide :puppetizer, :parent => :base do
  desc <<-'EOT'
    Service management for puppetizer.

    This provider manages daemons running supervised by Puppetizer Init.
    When detecting the service directory it will check

    * `/opt/puppetizer/etc/service`

    This provider supports:

    * start/stop
    # * restart
    * status

  EOT

  commands :init => '/opt/puppetizer/bin/init'

  def default
    true
  end

  def servicedir
    "/opt/puppetizer/etc/services"
  end

  def self.instances
    Dir.entries(self.servicedir).select { |e|
      e.end_with?".start"
    }.collect do |name|
      new(:name => name[0..-7])
    end
  end

  def start
    init(:start, @resource[:name], '--wait')
  end

  def stop
    init(:stop, @resource[:name], '--wait')
  end

  def status
    unless Facter.value('puppetizer')['running']
      return :stopped
    end

    output = %x[/opt/puppetizer/bin/init #{:status} #{@resource[:name]}].chomp!
    if ["UP", "PENDING UP"].include? output
      :running
    else
      :stopped
    end
  end
end
