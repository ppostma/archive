package nl.pointless.sudoku;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 * @author Peter Postma
 */
public class SudokuControlPanel extends JPanel implements ActionListener, SudokuListener
{
	private JButton buttonSolve;
	private JButton buttonReset;
	private JButton buttonClear;
	private JCheckBox checkDelay;
	private JLabel labelDelay;
	private JLabel labelText;

	private Sudoku sudoku;

	public SudokuControlPanel(Sudoku sudoku)
	{
		this.sudoku = sudoku;

		setSize(100, 400);
		setLayout(null);
		setVisible(true);

		buttonSolve = new JButton("Solve");
		buttonSolve.setBounds(5, 20, 80, 20);
		buttonSolve.setVisible(true);
		buttonSolve.setFocusable(false);
		buttonSolve.addActionListener(this);
		add(buttonSolve);

		buttonReset = new JButton("Reset");
		buttonReset.setBounds(5, 50, 80, 20);
		buttonReset.setVisible(true);
		buttonReset.setFocusable(false);
		buttonReset.addActionListener(this);
		add(buttonReset);

		buttonClear = new JButton("Clear");
		buttonClear.setBounds(5, 80, 80, 20);
		buttonClear.setVisible(true);
		buttonClear.setFocusable(false);
		buttonClear.addActionListener(this);
		add(buttonClear);

		checkDelay = new JCheckBox();
		checkDelay.setBounds(2, 110, 20, 20);
		checkDelay.setVisible(true);
		checkDelay.setFocusable(false);
		checkDelay.setSelected(false);
		checkDelay.addActionListener(this);
		add(checkDelay);

		labelDelay = new JLabel("add delay");
		labelDelay.setBounds(25, 110, 100, 20);
		labelDelay.setVisible(true);
		labelDelay.setFocusable(false);
		add(labelDelay);

		labelText = new JLabel();
		labelText.setBounds(2, 125, 95, 100);
		labelText.setVisible(true);
		labelText.setFocusable(false);
		add(labelText);
	}

	public void actionPerformed(ActionEvent e)
	{
		if (e.getSource() == buttonSolve) {
			if (sudoku.isSolving()) {
				sudoku.stopSolving();
			} else {
				if (sudoku.check()) {
					sudoku.startSolving();
				} else {
					labelText.setText("<html>Invalid sudoku.</html>");
				}
			}
		} else if (e.getSource() == buttonReset) {
			labelText.setText("");

			sudoku.reset();
		} else if (e.getSource() == buttonClear) {
			labelText.setText("");

			sudoku.clear();
		} else if (e.getSource() == checkDelay) {
			JCheckBox checkbox = (JCheckBox)e.getSource();

			if (checkbox.isSelected()) {
				sudoku.setDelay(true);
			} else {
				sudoku.setDelay(false);
			}
		}
	}

	public void sudokuUpdated()
	{
	}

	public void sudokuSolveStarted()
	{
		checkDelay.setEnabled(false);
		buttonClear.setEnabled(false);
		buttonReset.setEnabled(false);
		buttonSolve.setText("Stop");
		labelText.setText("");
	}

	public void sudokuSolveStopped()
	{
		checkDelay.setEnabled(true);
		buttonClear.setEnabled(true);
		buttonReset.setEnabled(true);
		buttonSolve.setText("Solve");
	}

	public void sudokuSolveFinished(boolean solved, long milliseconds)
	{
		if (solved) {
			labelText.setText("<html>Sudoku solved in " + (milliseconds / 1000.0) + " seconds.</html>");
		} else {
			labelText.setText("<html>Impossible to solve sudoku.</html>");
		}

		sudokuSolveStopped();
	}
}
