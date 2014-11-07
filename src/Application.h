#ifndef APPLICATION_H
#define APPLICATION_H

class Application {
 public:
    Application() { }

    virtual int acquireCriticalResources() { return 0; }

    virtual int acquireNonCriticalResources() { return 0; }

    virtual int step() { return 0; }

    virtual int releaseCriticalResources() { return 0; }
    
    virtual int releaseNonCriticalResources() { return 0; }
};

#endif