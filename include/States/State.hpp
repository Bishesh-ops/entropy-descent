#pragma once
class StateMachine;

class State
{
public:
    virtual ~State() = default;

    virtual void processInput() = 0;
    virtual void update(float dt) = 0;
    virtual void render() = 0;

    virtual void onEnter() {};
    virtual void onExit() {};
};