package nl.sogyo.mancala.ui;

import javax.swing.JFrame;
import javax.swing.JPanel;

import nl.sogyo.mancala.domein.Mancala;
import nl.sogyo.mancala.domein.interfaces.IMancala;

/**
 * @author Peter Postma
 */
public class MancalaFrame extends JFrame {

	public MancalaFrame(IMancala mancala) {
		setTitle("Mancala");
		setSize(580, 250);
		setResizable(false);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		MancalaPanel mancalaPanel = new MancalaPanel(mancala);
		mancalaPanel.setLocation(0, 0);
		mancalaPanel.setVisible(true);

		JPanel contentPane = (JPanel) getContentPane();
		contentPane.setVisible(true);
		contentPane.setLayout(null);
		contentPane.add(mancalaPanel);

		setVisible(true);
	}

	public static void main(String[] args) {
		// PlasticLookAndFeel.setPlasticTheme(new Silver());
		// try {
		// UIManager.setLookAndFeel(new PlasticXPLookAndFeel());
		// } catch (Exception e) {
		// System.out.println("Unable to set custom LAF: " + e.getMessage());
		// }

		new MancalaFrame(new Mancala("A", "B", 6, 4));
	}
}
