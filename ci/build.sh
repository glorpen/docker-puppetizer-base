#
# author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

for tag in ${REPO_TAGS};
do
	docker build --build-arg IMAGE_VERSION="${tag}" -t "${REPO_NAME}":"${tag}" .
done
