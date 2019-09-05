require 'puppet'

Facter.add(:puppetizer) do
  is_building = Puppet[:environment] == 'build'
  is_halting = Puppet[:environment] == 'halt'

  # is_halting = false
  # if !is_building
  #   output = %x[/opt/puppetizer/bin/init status].chomp!
  #   is_halting = output == "halting"
  # end

  setcode do
    {
      "building" => is_building,
      "running" => !is_building && !is_halting,
      "halting" => is_halting
    }
  end
end
