require 'puppet'

Facter.add(:puppetizer) do
  is_building = Puppet[:environment] == 'build'
  setcode do
    {
      "building" => is_building,
      "running" => !is_building
    }
  end
end
