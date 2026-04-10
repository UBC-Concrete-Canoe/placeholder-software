#pragma once

/**
 * @brief Abstract base class for all undoable commands.
 */
class Command
{
public:
	//! Destructor.
	virtual ~Command() = default;

	//! Execute the command.
	virtual void execute() = 0;

	//! Reverse the effect of the command.
	virtual void undo() = 0;
};
