import datetime
tm = datetime.datetime.today()

FILENAME_BUILDNO = 'versioning'
FILENAME_VERSIONNR = 'versionnr'
FILENAME_VERSION_H = 'include/version.h'


try:
    with open(FILENAME_VERSIONNR) as f:
        version_nr = str(f.readline())
except:
    print('Starting build number from 1..')
    version_nr = 'v0.1.'
with open(FILENAME_VERSIONNR, 'w+') as f:
    f.write(str(version_nr))
    print('Version number: {}'.format(version_nr))
version = str(version_nr) + str(tm.year)[-2:]+('0'+str(tm.month))[-2:]+('0'+str(tm.day))[-2:] + '_'

build_no = 0
try:
    with open(FILENAME_BUILDNO) as f:
        build_no = int(f.readline()) + 1
except:
    print('Starting build number from 1..')
    build_no = 1
with open(FILENAME_BUILDNO, 'w+') as f:
    f.write(str(build_no))
    print('Build number: {}'.format(build_no))

hf = """
#ifndef BUILD_NUMBER
  #define BUILD_NUMBER "{}"
#endif
#ifndef VERSION
  #define VERSION "{} - {}"
#endif
#ifndef VERSION_SHORT
  #define VERSION_SHORT "{}"
#endif
""".format(build_no, version+str(build_no), datetime.datetime.now(), version+str(build_no))
with open(FILENAME_VERSION_H, 'w+') as f:
    f.write(hf)
