package nl.pointless.sudoku;

import javax.swing.JFrame;
import javax.swing.JPanel;

/**
 * @author Peter Postma
 */
public class SudokuFrame extends JFrame
{
	private Sudoku sudoku = new Sudoku();

	public SudokuFrame()
	{
		setTitle("Sudoku solver");
		setSize(475, 400);
		setResizable(false);
		setFocusable(false);
		setFocusTraversalKeysEnabled(false);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		SudokuPanel sudokuPanel = new SudokuPanel(sudoku);
		sudokuPanel.setLocation(0, 0);
		sudokuPanel.setFocusable(true);
		sudokuPanel.setFocusTraversalKeysEnabled(false);
		sudokuPanel.setVisible(true);

		SudokuControlPanel sudokuControlPanel = new SudokuControlPanel(sudoku);
		sudokuControlPanel.setLocation(375, 0);
		sudokuControlPanel.setFocusable(false);
		sudokuControlPanel.setFocusTraversalKeysEnabled(false);
		sudokuControlPanel.setVisible(true);

		JPanel contentPane = (JPanel)getContentPane();
		contentPane.setVisible(true);
		contentPane.setLayout(null);
		contentPane.add(sudokuPanel);
		contentPane.add(sudokuControlPanel);

		sudoku.addEventListener(sudokuPanel);
		sudoku.addEventListener(sudokuControlPanel);

		setVisible(true);
	}

	public static void main(String[] args)
	{
		new SudokuFrame();
	}
}
