#
# Author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

set -e

puppetizer_sources="/opt/puppetizer/sources"
puppetizer_var_dir="/var/opt/puppetizer"
puppetizer_modules="${puppetizer_var_dir}/modules"
puppetizer_vendor_modules="/var/opt/puppetizer/vendor"
puppetizer_init="/var/opt/puppetizer/init.pp"
puppet_conf_dir="/etc/puppetlabs/puppet"
puppet_env_dir="/etc/puppetlabs/code/environments"

function puppet_apply()
{
	env="${1:-production}"
	
	# apply puppet manifests and check for exit code
	set +e
	/opt/puppetlabs/bin/puppet apply --detailed-exitcodes --verbose --environment=${env} --modulepath="${puppetizer_modules}:${puppetizer_vendor_modules}" "${puppetizer_init}"
	puppet_ret=$?
	set -e
	
	#return $puppet_ret
	
	if [ $puppet_ret == 2 ] || [ $puppet_ret == 0 ];
	then
		return 0
	else
		return 1
	fi
}
