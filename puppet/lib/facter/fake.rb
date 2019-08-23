Facter.add(:fqdn) do
  setcode do
    "docker"
  end
end
Facter.add(:ipaddress) do
  setcode do
    "127.0.0.1"
  end
end
Facter.add(:virtual) do
  setcode do
    "docker"
  end
end
