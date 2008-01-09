package nl.pointless.sudoku;

import java.util.LinkedList;
import java.util.List;

/**
 * @author Peter Postma
 */
public class Sudoku implements Runnable
{
	private static final int SIZE = 9;
	private static final int DELAY = 10;

	private List<SudokuListener> listeners;
	private Box boxes[][];

	private boolean stopSolving;
	private boolean isSolving;
	private boolean wantDelay;

	public Sudoku()
	{
		stopSolving = false;
		isSolving = false;
		wantDelay = false;

		listeners = new LinkedList<SudokuListener>();
		boxes = new Box[SIZE][SIZE];

		for (int row = 0; row < SIZE; row++) {
			for (int column = 0; column < SIZE; column++) {
				boxes[row][column] = new Box(column, row, 0);
			}
		}
	}

	/**
	 * Add an event listener.
	 */
	public void addEventListener(SudokuListener listener)
	{
		listeners.add(listener);
	}

	/**
	 * Enable or disable a delay during solving.
	 */
	public void setDelay(boolean delay)
	{
		wantDelay = delay;
	}

	/**
	 * Returns true if the sudoku is being solved.
	 */
	public boolean isSolving()
	{
		return isSolving;
	}

	/**
	 * Get the value of a box.
	 */
	public int getBoxValue(int column, int row)
	{
		if (row < 0 || row > 8 || column < 0 || column > 8) {
			throw new IllegalArgumentException("Invalid row or column given");
		}
		return boxes[row][column].getValue();
	}

	/**
	 * Set the value of a box.  This will only succeed when the box is mutable.
	 * Call {@link SudokuListener#sudokuUpdated()} afterwards.
	 */
	public void setBoxValue(int column, int row, int value)
	{
		if (row < 0 || row > 8 || column < 0 || column > 8) {
			throw new IllegalArgumentException("Invalid row or column given");
		}
		boxes[row][column].setValue(value);

		for (SudokuListener e: listeners) {
			e.sudokuUpdated();
		}
	}

	/**
	 * Set the value of a box and change the mutable state.
	 * 
	 * @see #setBoxValue(int, int, int)
	 * @see #setBoxMutable(int, int, boolean)
	 */
	public void setBoxValue(int column, int row, int value, boolean mutable)
	{
		setBoxValue(column, row, value);
		setBoxMutable(column, row, mutable);		
	}

	/**
	 * Remove the value of a box and put it into mutable state.
	 * Call {@link SudokuListener#sudokuUpdated()} afterwards.
	 */
	public void removeBoxValue(int column, int row)
	{
		if (row < 0 || row > 8 || column < 0 || column > 8) {
			throw new IllegalArgumentException("Invalid row or column given");
		}
		boxes[row][column].removeValue();
		boxes[row][column].setMutable(true);

		for (SudokuListener e: listeners) {
			e.sudokuUpdated();
		}
	}

	/**
	 * Set a box to the mutable state.
	 */
	public void setBoxMutable(int column, int row, boolean mutable)
	{
		if (row < 0 || row > 8 || column < 0 || column > 8) {
			throw new IllegalArgumentException("Invalid row or column given");
		}
		boxes[row][column].setMutable(mutable);
	}

	/**
	 * Returns the mutable state of a box.
	 */
	public boolean isBoxMutable(int column, int row)
	{
		if (row < 0 || row > 8 || column < 0 || column > 8) {
			throw new IllegalArgumentException("Invalid row or column given");
		}
		return boxes[row][column].isMutable();
	}

	/**
	 * Check if the given value exists in another column of the same row.
	 */
	private boolean isValueInRow(int row, int value)
	{
		for (int column = 0; column < SIZE; column++) {
			if (getBoxValue(column, row) == value) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Check if the given value exists in another row of the same column.
	 */
	private boolean isValueInColumn(int column, int value)
	{
		for (int row = 0; row < SIZE; row++) {
			if (getBoxValue(column, row) == value) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Check if the given value exists in the 3x3 box.
	 */
	private boolean isValueInBox(int column, int row, int value)
	{
		int startRow = (3 * (int)Math.floor(row / 3.0));
		int startColumn = (3 * (int)Math.floor(column / 3.0));

		for (int irow = startRow; irow < startRow + 3; irow++) {
			for (int icolumn = startColumn; icolumn < startColumn + 3; icolumn++) {
				if (getBoxValue(icolumn, irow) == value) {
					return true;
				}
			}
		}

		return false;
	}

	/**
	 * Try to solve the sudoku by using "backtracking".
	 * 
	 * @return true if the sudoku was solved, false otherwise
	 */
	private boolean solve()
	{
		boolean foundValue, solution = false, done = false;
		int row = 0, column = 0;

		/* Go to the first empty box and start solving from there. */
		while (getBoxValue(column, row) > 0) {
			if (++column > 8) {
				column = 0;
				if (++row > 8) {
					return true;
				}
			}
		}

		while (!done && !stopSolving) {
			foundValue = false;

			/* Pause if delay is enabled. */
			if (wantDelay) {
				try {
					Thread.sleep(Sudoku.DELAY);
				} catch (InterruptedException e) { }
			}

			/* If the box is mutable, try to fill it with the first possible value. */
			if (isBoxMutable(column, row)) {
				for (int value = getBoxValue(column, row) + 1; value <= SIZE; value++) {
					if (isValueInRow(row, value)) {
						continue;
					}
					if (isValueInColumn(column, value)) {
						continue;
					}
					if (isValueInBox(column, row, value)) {
						continue;
					}
					setBoxValue(column, row, value);
					foundValue = true;
					break;
				}
			} else {
				foundValue = true;
			}

			/*
			 * If no value was found, then go back to the first mutable box and
			 * continue solving.
			 * If a value was found, then go to the next box. 
			 */
			if (!foundValue) {
				removeBoxValue(column, row);

				boolean exitLoop = false;
				do {
					if (--column < 0) {
						if (--row < 0) {
							exitLoop = true;
							solution = false;
							done = true;
						} else {
							column = 8;
						}
					}
					if (!done && isBoxMutable(column, row)) {
						exitLoop = true;
					}
				} while (!exitLoop);
			} else {
				if (++column > 8) {
					if (++row > 8) {
						solution = true;
						done = true;
					} else {
						column = 0;
					}
				}
			}
		}

		return solution;
	}

	/**
	 * Start solving the sudoku.
	 * This method just fires off a thread to do the work.
	 */
	public void startSolving()
	{
		stopSolving = false;

		Thread thread = new Thread(this);
		thread.setDaemon(true);
		thread.setName("Sudoku solver");
		thread.start();
	}

	/**
	 * Stop solving of the sudoku.
	 */
	public void stopSolving()
	{
		stopSolving = true;
	}

	/**
	 * Check if the sudoku is valid: doesn't contain double values in the rows,
	 * columns and 3x3 boxes.
	 * 
	 * @return true if valid, false otherwise
	 */
	public boolean check()
	{
		for (int row = 0; row < SIZE; row++) {
			for (int column = 0; column < SIZE; column++) {
				if (getBoxValue(column, row) == 0) {
					continue;
				}

				/* Check if the value exists in another row. */
				for (int irow = 0; irow < SIZE; irow++) {
					if (row == irow) {
						continue;
					}
					if (getBoxValue(column, irow) == 0) {
						continue;
					}
					if (getBoxValue(column, row) == getBoxValue(column, irow)) {
						return false;
					}
				}

				/* Check if the value exists in another column. */
				for (int icolumn = 0; icolumn < SIZE; icolumn++) {
					if (column == icolumn) {
						continue;
					}
					if (getBoxValue(icolumn, row) == 0) {
						continue;
					}
					if (getBoxValue(column, row) == getBoxValue(icolumn, row)) {
						return false;
					}
				}

				/* Check if the value exists in the "box". */
				int startRow = (3 * (int)Math.floor(row / 3.0));
				int startColumn = (3 * (int)Math.floor(column / 3.0));

				for (int irow = startRow; irow < startRow + 3; irow++) {
					for (int icolumn = startColumn; icolumn < startColumn + 3; icolumn++) {
						if (column == icolumn && row == irow) {
							continue;
						}
						if (getBoxValue(icolumn, irow) == 0) {
							continue;
						}
						if (getBoxValue(column, row) == getBoxValue(icolumn, irow)) {
							return false;
						}
					}
				}
			}
		}
		return true;
	}

	/**
	 * Reset the "solved" values in the sudoku. 
	 */
	public void reset()
	{
		for (int row = 0; row < SIZE; row++) {
			for (int column = 0; column < SIZE; column++) {
				if (isBoxMutable(column, row)) {
					removeBoxValue(column, row);
				}
			}
		}
	}

	/**
	 * Clear all values in the sudoku.
	 */
	public void clear()
	{
		for (int row = 0; row < SIZE; row++) {
			for (int column = 0; column < SIZE; column++) {
				removeBoxValue(column, row);
			}
		}
	}

	/**
	 * Start solving of the sudoku.
	 */
	public void run()
	{
		isSolving = true;

		for (SudokuListener e: listeners) {
			e.sudokuSolveStarted();
		}

		long start = System.currentTimeMillis();
		boolean solved = solve(); 
		long stop = System.currentTimeMillis();

		if (stopSolving) {
			for (SudokuListener e: listeners) {
				e.sudokuSolveStopped();
			}
		} else {
			for (SudokuListener e: listeners) {
				e.sudokuSolveFinished(solved, (stop - start));
			}
		}

		isSolving = false;
	}
}
