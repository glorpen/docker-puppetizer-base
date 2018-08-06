#
# author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

function list_tags(){
	if [ "${TRAVIS_BRANCH}" == "master" ];
	then
		suffix="latest";
	else
		suffix="${TRAVIS_TAG/v}";
	fi
	
	if [ "x${suffix}" != "x" ];
	then
		find ./ -maxdepth 1 -name '*.dockerfile' -printf '%f\n' | sort -fdr | while read n;
		do
			echo "${n}" | sed -e "s/.dockerfile/-${suffix}/" -e "s/^/${n}:/"
		done
	fi
}

function list_tags_only(){
	list_tags | cut -d: -f2
}
