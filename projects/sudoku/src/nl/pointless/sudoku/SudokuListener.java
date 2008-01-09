package nl.pointless.sudoku;

public interface SudokuListener
{
	void sudokuUpdated();
	void sudokuSolveStarted();
	void sudokuSolveStopped();
	void sudokuSolveFinished(boolean solved, long milliseconds);
}
