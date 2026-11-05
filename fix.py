import re
p='tools/trantp_probe.cpp'
s=open(p).read()
start=s.index('extern "C" {')
end=s.index('}\n', s.index('void AParcel_delete(AParcel* parcel);'))+2
s = s[:start] + 'extern "C" {\nAIBinder* AServiceManager_getService(const char* instance);\n}\n\n' + s[end:]
s = s.replace('#include <android/binder_parcel.h>', '#include <android/binder_parcel.h>\n#include <android/binder_status.h>')
open(p,'w').write(s)
print("patched")
