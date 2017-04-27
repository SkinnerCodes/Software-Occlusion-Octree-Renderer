class StopWatch
{
private:
    unsigned long long nBefore;
    unsigned long long nAfter;

public:
    double GetDurationMilis() {
        unsigned long long nFreq = GetPerformanceTicksInSecond();
        const unsigned long long nDiff = nAfter - nBefore;
        const unsigned long long nMicroseconds = GetTickMiliseconds(nDiff,nFreq);
    }
    void Start() {
        unsigned long long nBefore = GetPerformanceTicks();
    }
    void Stop()  {
        nAfter = GetPerformanceTicks();
        const unsigned long long nDiff = nAfter - nBefore;
    }
    unsigned long long GetPerformanceTicks()
    {
        LARGE_INTEGER nValue;

        QueryPerformanceCounter(&nValue);

        return nValue.QuadPart;
    }

    unsigned long long GetPerformanceTicksInSecond()
    {
        LARGE_INTEGER nFreq;

        QueryPerformanceFrequency(&nFreq);

        return nFreq.QuadPart;
    }

    double GetTickSeconds(unsigned long long nTicks,unsigned long long nFreq)
    {
        return static_cast<double>(nTicks) / static_cast<double>(nFreq);
    }

    unsigned long long GetTickMilliseconds(unsigned long long nTicks,unsigned long long nFreq)
    {
        unsigned long long nTicksInMillisecond = nFreq / 1000;

        return nTicks / nTicksInMillisecond;
    }

    unsigned long long GetTickMiliseconds(unsigned long long nTicks,unsigned long long nFreq)
    {
        unsigned long long nTicksInMicrosecond = nFreq / 1000;

        return nTicks / nTicksInMicrosecond;
    }
}; //