package nl.pointless.sudoku;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;

import javax.swing.JPanel;

/**
 * @author Peter Postma
 */
public class SudokuPanel extends JPanel implements MouseListener, KeyListener, SudokuListener
{
	private static final int BASE_X = 8;
	private static final int BASE_Y = 8;
	private static final int BOX_WIDTH = 40;

	private Point selectedBox;
	private boolean controlsEnabled;

	private Sudoku sudoku;

	public SudokuPanel(Sudoku sudoku)
	{
		this.sudoku = sudoku;
		this.controlsEnabled = true;

		setSize(375, 400);
		setLayout(null);
		setVisible(true);

		requestFocus();

		addMouseListener(this);
		addKeyListener(this);
	}

	private void fillSudoku(int number)
	{
		switch (number) {
		case 1:
			/* Easy sudoku from Sp!ts. */
			sudoku.setBoxValue(1, 0, 1, false);
			sudoku.setBoxValue(4, 0, 6, false);
			sudoku.setBoxValue(8, 0, 7, false);
			sudoku.setBoxValue(2, 1, 4, false);
			sudoku.setBoxValue(4, 1, 8, false);
			sudoku.setBoxValue(5, 1, 3, false);
			sudoku.setBoxValue(8, 1, 2, false);
			sudoku.setBoxValue(0, 2, 8, false);
			sudoku.setBoxValue(1, 2, 2, false);
			sudoku.setBoxValue(3, 2, 1, false);
			sudoku.setBoxValue(7, 2, 5, false);
			sudoku.setBoxValue(8, 2, 3, false);
			sudoku.setBoxValue(0, 3, 4, false);
			sudoku.setBoxValue(1, 3, 8, false);
			sudoku.setBoxValue(7, 3, 3, false);
			sudoku.setBoxValue(1, 4, 9, false);
			sudoku.setBoxValue(2, 4, 3, false);
			sudoku.setBoxValue(4, 4, 7, false);
			sudoku.setBoxValue(6, 4, 1, false);
			sudoku.setBoxValue(0, 5, 7, false);
			sudoku.setBoxValue(4, 5, 9, false);
			sudoku.setBoxValue(5, 5, 1, false);
			sudoku.setBoxValue(2, 6, 8, false);
			sudoku.setBoxValue(3, 6, 6, false);
			sudoku.setBoxValue(8, 6, 1, false);
			sudoku.setBoxValue(2, 7, 7, false);
			sudoku.setBoxValue(4, 7, 2, false);
			sudoku.setBoxValue(5, 7, 4, false);
			sudoku.setBoxValue(6, 7, 5, false);
			sudoku.setBoxValue(8, 7, 6, false);
			sudoku.setBoxValue(1, 8, 6, false);
			sudoku.setBoxValue(2, 8, 9, false);
			sudoku.setBoxValue(7, 8, 4, false);
			break;
		case 2:
			/* Difficult sudoku from a Saturday edition of Trouw. */
			sudoku.setBoxValue(2, 0, 7, false);
			sudoku.setBoxValue(7, 0, 1, false);
			sudoku.setBoxValue(0, 1, 8, false);
			sudoku.setBoxValue(3, 1, 1, false);
			sudoku.setBoxValue(6, 1, 6, false);
			sudoku.setBoxValue(7, 1, 9, false);
			sudoku.setBoxValue(1, 2, 5, false);
			sudoku.setBoxValue(8, 3, 6, false);
			sudoku.setBoxValue(0, 4, 2, false);
			sudoku.setBoxValue(4, 4, 5, false);
			sudoku.setBoxValue(4, 5, 2, false);
			sudoku.setBoxValue(5, 5, 4, false);
			sudoku.setBoxValue(6, 5, 7, false);
			sudoku.setBoxValue(1, 6, 4, false);
			sudoku.setBoxValue(4, 6, 6, false);
			sudoku.setBoxValue(5, 6, 3, false);
			sudoku.setBoxValue(8, 6, 7, false);
			sudoku.setBoxValue(3, 7, 8, false);
			sudoku.setBoxValue(1, 8, 9, false);
			sudoku.setBoxValue(2, 8, 1, false);
			sudoku.setBoxValue(5, 8, 7, false);
			sudoku.setBoxValue(7, 8, 8, false);
			break;
		default:
			return;
		}
	}

	public void paintComponent(Graphics g)
	{
		super.paintComponent(g);

		/* Draw the selection. */
		if (selectedBox != null) {
			g.setColor(Color.LIGHT_GRAY);
			g.fillRect(BASE_X + (selectedBox.x * BOX_WIDTH),
					BASE_Y + (selectedBox.y * BOX_WIDTH), BOX_WIDTH, BOX_WIDTH);
		}

		g.setColor(Color.BLACK);

		/* Draw the lines. */
		for (int row = 0; row < 9; row++) {
			for (int column = 0; column < 9; column++) {
				g.drawRect(BASE_X + (column * BOX_WIDTH),
						BASE_Y + (row * BOX_WIDTH), BOX_WIDTH, BOX_WIDTH);
			}
		}

		/* Draw the thick lines. */
		for (int column = 0; column <= 9; column += 3) {
			g.drawLine((BASE_X - 1) + (column * BOX_WIDTH), BASE_Y,
					(BASE_X - 1) + (column * BOX_WIDTH), BASE_Y + (BOX_WIDTH * 9));
			g.drawLine((BASE_X + 1) + (column * BOX_WIDTH), BASE_Y,
					(BASE_X + 1) + (column * BOX_WIDTH), BASE_Y + (BOX_WIDTH * 9));
		}
		for (int row = 0; row <= 9; row += 3) {
			g.drawLine(BASE_X, (BASE_Y - 1) + (row * BOX_WIDTH),
					BASE_X + (BOX_WIDTH * 9), (BASE_Y - 1) + (row * BOX_WIDTH));
			g.drawLine(BASE_X, (BASE_Y + 1) + (row * BOX_WIDTH),
					BASE_X + (BOX_WIDTH * 9), (BASE_Y + 1) + (row * BOX_WIDTH));
		}

		g.setFont(new Font("SansSerif", Font.BOLD, 16));

		/* Draw the values. */
		for (int row = 0; row < 9; row++) {
			for (int column = 0; column < 9; column++) {
				int value = sudoku.getBoxValue(column, row);

				if (value != 0) {
					if (sudoku.isBoxMutable(column, row)) {
						g.setColor(Color.BLUE);
					} else {
						g.setColor(Color.RED);
					}

					g.drawString(String.valueOf(value),
							BASE_X + (column * BOX_WIDTH) + 15,
							BASE_Y + (row * BOX_WIDTH) + 25);
				}
			}
		}
	}

	public void mouseClicked(MouseEvent e)
	{
	}

	public void mouseEntered(MouseEvent e)
	{
	}

	public void mouseExited(MouseEvent e)
	{
	}

	public void mousePressed(MouseEvent e)
	{
		if (!controlsEnabled) {
			return;
		}

		if (e.getButton() == 1) {
			if ((e.getX() - BASE_X) < 0 || (e.getY() - BASE_Y) < 0) {
				/* Out of range. */
				return;
			}

			int column = (e.getX() - BASE_X) / BOX_WIDTH;
			int row = (e.getY() - BASE_Y) / BOX_WIDTH;

			if (column < 0 || column > 8 || row < 0 || row > 8) {
				/* Out of range. */
				return;
			}

			if (selectedBox != null && selectedBox.x == column && selectedBox.y == row) {
				selectedBox = null;
			} else {
				selectedBox = new Point(column, row);
			}

			repaint();
		}
	}

	public void mouseReleased(MouseEvent e)
	{
	}

	public void keyPressed(KeyEvent e)
	{
		if (!controlsEnabled) {
			return;
		}

		switch (e.getKeyCode()) {
		case KeyEvent.VK_1:
		case KeyEvent.VK_2:
		case KeyEvent.VK_3:
		case KeyEvent.VK_4:
		case KeyEvent.VK_5:
		case KeyEvent.VK_6:
		case KeyEvent.VK_7:
		case KeyEvent.VK_8:
		case KeyEvent.VK_9:
			if (selectedBox != null) {
				int value = Character.getNumericValue(e.getKeyChar());
				sudoku.setBoxMutable(selectedBox.x, selectedBox.y, true);
				sudoku.setBoxValue(selectedBox.x, selectedBox.y, value, false);
				selectedBox = null;
			}
			break;
		case KeyEvent.VK_DELETE:
			if (selectedBox != null) {
				sudoku.removeBoxValue(selectedBox.x, selectedBox.y);
				selectedBox = null;
			}
			break;
		case KeyEvent.VK_NUMPAD1:
			sudoku.clear();
			fillSudoku(1);
			break;
		case KeyEvent.VK_NUMPAD2:
			sudoku.clear();
			fillSudoku(2);
			break;
		}
	}

	public void keyReleased(KeyEvent e)
	{
	}
	
	public void keyTyped(KeyEvent e)
	{
	}

	public void sudokuUpdated()
	{
		repaint();
	}

	public void sudokuSolveStarted()
	{
		controlsEnabled = false;
	}

	public void sudokuSolveStopped()
	{
		controlsEnabled = true;
	}

	public void sudokuSolveFinished(boolean solved, long milliseconds)
	{
		sudokuSolveStopped();
	}
}

