#include <utilities/log.h>
#include <utilities/counter.h>

static Counter* cTestElaborate = GetCounter("test/elaborate");
static Counter* cTestBasic = GetCounter("test/basic");
static Counter* cTestBasicSub = GetCounter("test/basic/sub");

int main()
{
	SetFrequency(cTestBasic, 2);

	Count(cTestBasic);
	Count(cTestBasicSub);
	Count(cTestBasicSub);
	Count(cTestBasicSub);
	Count(cTestElaborate);
	Count(cTestBasic);
	Count(cTestElaborate);
	Count(cTestBasic);
	Count(cTestBasic);
	
	SetFrequency(cTestElaborate, 3);
	Count(cTestElaborate);
}
