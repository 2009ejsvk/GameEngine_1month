#pragma once

class CMainWorld;

class CWorldSubsystem
{
public:
    explicit CWorldSubsystem(CMainWorld* Owner)
        : mOwner(Owner)
    {
    }

    virtual ~CWorldSubsystem() = default;

protected:
    CMainWorld* mOwner;
};
