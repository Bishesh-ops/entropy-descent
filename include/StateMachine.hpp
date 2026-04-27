#pragma once
#include <stack>
#include <memory>
#include "States/State.hpp"

class StateMachine
{
private:
    std::stack<std::unique_ptr<State>> states;
    bool isAdding;
    bool isReplacing;
    bool isRemoving;
    std::unique_ptr<State> newState;

public:
    StateMachine() : isAdding(false), isReplacing(false), isRemoving(false) {}

    void pushState(std::unique_ptr<State> state, bool replace = false)
    {
        isAdding = true;
        isReplacing = replace;
        newState = std::move(state);
    }

    void popState()
    {
        isRemoving = true;
    }

    void processStateChanges()
    {
        if (isRemoving && !states.empty())
        {
            states.top()->onExit();
            states.pop();
            if (!states.empty())
            {
                states.top()->onEnter();
            }
            isRemoving = false;
        }
        if (isAdding)
        {
            if (!states.empty())
            {
                if (isReplacing)
                {
                    states.top()->onExit();
                    states.pop();
                }
            }
            states.push(std::move(newState));
            states.top()->onEnter();
            isAdding = false;
        }
    }

    State &getActiveState()
    {
        return *states.top();
    }
    bool isEmpty() const { return states.empty(); }
};