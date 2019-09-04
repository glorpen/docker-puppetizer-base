require 'puppet'

Facter.add(:puppetizer) do
  is_building = Puppet[:environment] == 'build'
  is_halting = false

  if !is_building
    # try..catch
    # TODO: run init status => check if OUTPUT is "halting"
  end

  setcode do
    {
      "building" => is_building,
      "running" => !is_building,
      "halting" => is_halting
    }
  end
end
