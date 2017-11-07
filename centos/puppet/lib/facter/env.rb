Facter.add(:env) do
  default_vars=[
    "HOME", "PATH", "PWD", "TERM", "OLDPWD", "LS_COLORS", "LESSOPEN", "_", "RUBYOPT", "SHLVL", "HOSTNAME"
  ]
  setcode do
    ENV.to_hash.keep_if{
      | key, value |
      ! default_vars.include? key
    }
  end
end
