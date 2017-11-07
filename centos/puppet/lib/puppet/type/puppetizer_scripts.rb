Puppet::Type.newtype(:puppetizer_scripts) do
  @doc = <<-EOS
    Dump registered service scripts.
  EOS

  newparam(:name) do
    desc "The name."
    isnamevar
  end
  
  newparam(:service_scripts) do
    desc "Target dir to store service scripts"
    
    defaultto :undef
    
    validate do | value |
      if not value.is_a?(String)
        raise ArgumentError, "puppetizer_scripts target should be path to directory for puppetizer_scripts['%{name}']" % { name: name }
      end
      raise ArgumentError, "Path should be absolute" unless Puppet::Util.absolute_path?(value)
    end
  end

  def supported_services
    catalog.resources.collect.select do |r|
      r.is_a?(Puppet::Type.type(:service)) && r[:provider] == :base
    end
  end
  
  autorequire(:service) do
    supported_services.map do |r|
      r.name
    end
  end
  
  def generate
    [generate_service_scripts].flatten
  end
  
  def generate_service_scripts
    scripts_dir = self[:service_scripts]
      
    r = supported_services.map do |r|
      
      if r[:status] == nil or r[:stop] == nil or r[:start] == nil
        raise Exception, 'Service "%{service}" should have defined commands for "start", "stop" and "status" actions' % { service: r[:name] }
      end
      
      ret = ["start", "status", "stop"].map do |action|
        Puppet::Type.type(:file).new(
          :name => "%{target}/%{service}.%{action}" % { service: r[:name], target: scripts_dir, action: action },
          :ensure => :file,
          :content => "#!/bin/bash -e\n%{cmd}\n" % { cmd: r[action.to_s] },
          :backup => false,
          :mode => 'a=,u+rx'
        )
      end
      
      restart_cmd = if r[:restart] == nil
        "#!/bin/bash -e\n%{target}/%{service}.stop\n%{target}/%{service}.start\n" % { service: r[:name], target: scripts_dir }
      else
        "#!/bin/bash -e\n%{cmd}\n" % { cmd: r[:restart] }
      end
      
      ret << Puppet::Type.type(:file).new(
        :name => "%{target}/%{service}.restart" % { service: r[:name], target: scripts_dir},
        :ensure => :file,
        :content => restart_cmd,
        :backup => false,
        :mode => 'a=,u+rx'
      )
      
      ret
    end.flatten
  end
end
