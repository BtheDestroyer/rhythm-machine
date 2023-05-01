#pragma once
class Machine;

class State
{
public:
    virtual void operator()(Machine& machine) = 0;
};
