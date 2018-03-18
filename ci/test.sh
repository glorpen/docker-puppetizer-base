#
# author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

OUTPUT_PATH=/tmp/output
PUPPETFILE=/tmp/Puppetfile
CACHE_PATH=/tmp/cache
FAIL_LOCK=/tmp/.unit-fail

function run(){
	:;
}

function clean(){
	rm -rf "${OUTPUT_PATH}" "${CACHE_PATH}" "${FAIL_LOCK}"
}

function _fail(){
	touch "${FAIL_LOCK}"
}

function _exit2str(){
	if [ ${1} -eq 0 ];
	then
		echo -n "ok"
	else
		echo -n "failed"
	fi
}

function check_exists(){
	test -e "${1}"
	ret=$?
	[ $ret -ne 0 ] && _fail
	echo "## ${2:-Checking ${1}}.. $(_exit2str $ret)"
}
function check_exists_not(){
	test ! -e "${1}"
	ret=$?
	[ $ret -ne 0 ] && _fail
	echo "## ${2:-Inverse checking ${1}}.. $(_exit2str $ret)"
}

function check_cache(){
	check_exists "${CACHE_PATH}/${1}" "${2}"
}
function check_output(){
	check_exists "${OUTPUT_PATH}/${1}" "${2}"
}
function check_output_not(){
	check_exists_not "${OUTPUT_PATH}/${1}" "${2}"
}

# tests

# end tests

if [ -e "${FAIL_LOCK}" ];
then
	echo "!! Some tests failed"
	exit 1
else
	echo "-- All tests passed"
	clean
fi
