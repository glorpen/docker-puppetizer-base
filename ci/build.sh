#
# author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

set -e

REPO_REGEX="$(ls *.dockerfile | cut -d. -f1 | sort -r | sed -e 's@\(.*\)@^\\\\(\1\\\\)-.*$@')"

for tag in ${REPO_TAGS};
do
	prefix=$(echo "${REPO_REGEX}" | while read n; do echo "${tag}" | sed -n "s/$n/\1/p"; done | head -n1)
	docker build --build-arg IMAGE_VERSION="${tag}" -t "${REPO_NAME}":"${tag}" -f "${prefix}.dockerfile" .
done
