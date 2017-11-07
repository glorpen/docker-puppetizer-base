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

puppetizer_conf_dir="/etc/puppetizer.d"
puppetizer_health_dir="${puppetizer_conf_dir}/health"
puppetizer_services_dir="${puppetizer_conf_dir}/services"
puppetizer_scripts_dir="${puppetizer_conf_dir}/scripts"
puppetizer_initialized_token="${puppetizer_var_dir}/initialized"

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

function find_scripts()
{
	path="${1}"; shift
	find "${path}" -type f -perm -u+x $@ | sort
}

function run_scripts_sync()
{
	while read n
	do
		echo "Executing ${n} ...";
		"${n}" || echo "Running ${n} failed with ${?}, continuing";
	done
}
