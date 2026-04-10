#pragma once

#include <memory>
#include <stack>

#include "Command.h"

/**
 * @brief Manages command execution with undo and redo support.
 */
class HistoryManager
{
public:
	//! Maximum number of commands retained in the undo stack.
	static constexpr int MAX_HISTORY = 100;

	/**
	 * @brief Execute a command and push it onto the undo stack.
	 * @param cmd The command to execute.
	 */
	void execute(std::shared_ptr<Command> cmd);

	//! Undo the most recent command.
	void undo();

	//! Redo the most recently undone command.
	void redo();

	//! Returns true if there is at least one command to undo.
	bool canUndo() const;

	//! Returns true if there is at least one command to redo.
	bool canRedo() const;

	//! Clear both the undo and redo stacks.
	void clear();

private:
	std::stack<std::shared_ptr<Command>> undoStack;
	std::stack<std::shared_ptr<Command>> redoStack;
};
