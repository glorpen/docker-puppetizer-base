#
# Author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

set -e

puppetizer_bin="/opt/puppetizer/bin"
puppetizer_sources="/opt/puppetizer/sources"
puppetizer_var_dir="/var/opt/puppetizer"
puppetizer_modules="${puppetizer_var_dir}/modules"
puppetizer_vendor_modules="/var/opt/puppetizer/vendor"
puppetizer_init="/var/opt/puppetizer/init.pp"
puppet_conf_dir="/etc/puppetlabs/puppet"
puppet_env_dir="/etc/puppetlabs/code/environments"
puppet_modules_dir="/etc/puppetlabs/code/modules"
puppetizer_share_dir="/opt/puppetizer/share"

puppetizer_health_dir="${puppetizer_var_dir}/health"
puppetizer_services_dir="${puppetizer_var_dir}/services"
puppetizer_scripts_dir="${puppetizer_var_dir}/scripts"
puppetizer_initialized_token="${puppetizer_var_dir}/initialized"

puppetizer_os="$(cat $puppetizer_share_dir/os)"

puppetizer_has_feature(){
	lines=$(grep -xc "${1}" "${puppetizer_share_dir}/features")
	
	if [ $lines -eq 0 ];
	then
		return 1
	else
		return 0
	fi
}

puppet_apply()
{
	env="${1:-production}"
	if [ "x$PUPPETIZER_DEBUG" == "xy" ] || [ "x${env}" == "xbuild" ];
	then
		debug_opts="--verbose"
	else
		debug_opts="--log_level warning"
	fi
	
	# apply puppet manifests and check for exit code
	set +e
	"${puppetizer_bin}/puppet" apply --detailed-exitcodes $debug_opts --environment=${env} --modulepath="${puppetizer_modules}:${puppetizer_vendor_modules}:${puppet_modules_dir}" "${puppetizer_init}"
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

find_scripts()
{
	path="${1}"; shift
	find "${path}" -type f -perm -u+x $@ | sort
}

run_scripts_sync()
{
	while read n
	do
		echo "Executing ${n} ...";
		"${n}" || echo "Running ${n} failed with ${?}, continuing";
	done
}
