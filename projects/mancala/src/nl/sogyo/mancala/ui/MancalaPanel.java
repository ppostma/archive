package nl.sogyo.mancala.ui;

import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import nl.sogyo.mancala.domein.excepties.MancalaException;
import nl.sogyo.mancala.domein.interfaces.IKom;
import nl.sogyo.mancala.domein.interfaces.IMancala;
import nl.sogyo.mancala.domein.interfaces.ISpeler;
import nl.sogyo.mancala.domein.interfaces.ISpelerListener;

/**
 * @author Peter Postma
 */
public class MancalaPanel extends JPanel implements ActionListener, ISpelerListener
{
	private class KomEventHandler implements EventHandlerObject 
	{
		private IKom kom;
		private int aantalStenen;
		private boolean isSlag;

		public KomEventHandler(IKom kom, boolean isSlag)
		{
			this.kom = kom;
			this.aantalStenen = kom.getAantalStenen();
			this.isSlag = isSlag;
		}

		public void handleEvent()
		{
			JButton button = kommetjes.get(this.kom);
			button.setText(String.valueOf(this.aantalStenen));

			if (this.isSlag) {
				button.setBackground(Color.RED);
			} else {
				button.setBackground(Color.LIGHT_GRAY);
			}

			button.repaint();

			try {
				Thread.sleep(400);
			} catch (InterruptedException e) {
				// Ignore
			}
		}
	}

	private class SpelAfgelopenEventHandler implements EventHandlerObject
	{
		private Component parentComponent;

		public SpelAfgelopenEventHandler(Component parentComponent)
		{
			this.parentComponent = parentComponent;
		}

		public void handleEvent()
		{
			controlsEnabled = false;

			for (JButton b : kommetjes.values()) {
				b.setEnabled(false);
				b.repaint();
			}

			ISpeler winnaar = mancala.getWinnaar();

			if (winnaar == null) {
				JOptionPane.showMessageDialog(this.parentComponent,
						"Het spel is afgelopen.\n" +
						"Het is gelijk spel geworden, beide spelers hebben " +
						mancala.getSpeler1().getTotaalAantalStenen() + " stenen.",
						 "Spel afgelopen", JOptionPane.INFORMATION_MESSAGE);
			} else {
				JOptionPane.showMessageDialog(this.parentComponent,
						"Het spel is afgelopen.\n" +
						"De winnaar is speler " + winnaar.getNaam() + " met " +
						winnaar.getTotaalAantalStenen() + " stenen.",
						 "Spel afgelopen", JOptionPane.INFORMATION_MESSAGE);
			}
		}
	}

	private class SpelerWisselBeurtEventHandler implements EventHandlerObject
	{
		private ISpeler speler;

		public SpelerWisselBeurtEventHandler(ISpeler speler)
		{
			this.speler = speler;
		}

		public void handleEvent()
		{
			try {
				Thread.sleep(800);
			} catch (InterruptedException e) {
				// Ignore
			}

			for (JButton b : kommetjes.values()) {
				b.setBackground(Color.ORANGE);
				b.repaint();
			}

			messagesLabel.setText("Speler " + this.speler.getNaam() + " is nu aan de beurt.");
			messagesLabel.repaint();

			controlsEnabled = true;
		}
	}

	private IMancala mancala;
	private Map<IKom, JButton> kommetjes;
	private JLabel messagesLabel;
	private EventHandler eventHandler;
	private boolean controlsEnabled;

	public MancalaPanel(IMancala mancala)
	{
		this.mancala = mancala;
		this.mancala.addListener(this);

		this.kommetjes = new HashMap<IKom, JButton>();
		this.eventHandler = new EventHandler();

		this.controlsEnabled = true;

		setSize(580, 250);
		setLayout(null);
		setCursor(new Cursor(Cursor.HAND_CURSOR));
		
		ISpeler speler1 = mancala.getSpeler1();
		ISpeler speler2 = mancala.getSpeler2();

		// Maak speler 1 kommetjes en kalaha.
		List<IKom> kommetjesSpeler1 = speler1.getKommetjes();

		for (int i = 0; i < kommetjesSpeler1.size(); i++) {
			IKom kom = kommetjesSpeler1.get(i);

			kommetjes.put(kom, maakButton(speler1, i, (i * 70) + 80, 120,
					kom.getAantalStenen(), false));
		}

		IKom kalahaSpeler1 = speler1.getKalaha();
		kommetjes.put(kalahaSpeler1, maakButton(speler1, 0, (kommetjesSpeler1.size() * 70) + 80,
				70, kalahaSpeler1.getAantalStenen(), true));

		// Maak speler 2 kommetjes en kalaha.
		List<IKom> kommetjesSpeler2 = speler2.getKommetjes();

		for (int i = kommetjesSpeler2.size(); i > 0; i--) {
			IKom kom = kommetjesSpeler2.get(kommetjesSpeler2.size() - i);

			kommetjes.put(kom, maakButton(speler2, kommetjesSpeler2.size() - i,
					(i * 70) + 10, 20, kom.getAantalStenen(), false));
		}

		IKom kalahaSpeler2 = speler2.getKalaha();
		kommetjes.put(kalahaSpeler2, maakButton(speler2, 0, 10, 70,
				kalahaSpeler2.getAantalStenen(), true));

		// Label voor speler 1
		JLabel speler1Label = new JLabel(this.mancala.getSpeler1().getNaam());
		speler1Label.setLocation(520, 135);
		speler1Label.setSize(50, 20);
		speler1Label.setFont(new Font("arial", Font.BOLD, 24));
		speler1Label.setVisible(true);
		add(speler1Label);

		// Label voor speler 2
		JLabel speler2Label = new JLabel(this.mancala.getSpeler2().getNaam());
		speler2Label.setLocation(30, 25);
		speler2Label.setSize(50, 20);
		speler2Label.setFont(new Font("arial", Font.BOLD, 24));
		speler2Label.setVisible(true);
		add(speler2Label);

		// Label voor berichten
		messagesLabel = new JLabel("Speler " + this.mancala.getSpelerAanZet().getNaam() + " is nu aan de beurt.");
		messagesLabel.setLocation(30, 150);
		messagesLabel.setSize(580, 80);
		messagesLabel.setFont(new Font("arial", Font.BOLD, 16));
		messagesLabel.setVisible(true);
		add(messagesLabel);
	}

	public void actionPerformed(ActionEvent e)
	{
		if (!controlsEnabled) {
			return;
		}

		if (e.getSource() instanceof JButton) {
			JButton button = (JButton)e.getSource();
			ISpeler speler = (ISpeler)button.getClientProperty("speler");
			Integer positie = (Integer)button.getClientProperty("positie");

			try {
				IKom kom = speler.getKomOpPositie(positie);
				kom.speel();

				controlsEnabled = false;
			} catch (MancalaException ex) {
				JOptionPane.showMessageDialog(this, ex.getMessage(),
						"Mancala fout", JOptionPane.WARNING_MESSAGE);
			}
		}
	}

	public void komUpdateEvent(IKom kom, boolean isSlag)
	{
		this.eventHandler.addEvent(new KomEventHandler(kom, isSlag));
	}

	public void spelAfgelopenEvent()
	{
		this.eventHandler.addEvent(new SpelAfgelopenEventHandler(this));
	}

	public void spelerWisselBeurtEvent(ISpeler speler)
	{
		this.eventHandler.addEvent(new SpelerWisselBeurtEventHandler(speler));
	}

	private JButton maakButton(ISpeler speler, int positie, int x, int y,
			int aantalStenen, boolean isKalaha)
	{
		JButton button = new JButton();
		button.putClientProperty("speler", speler);
		button.putClientProperty("positie", positie);
		button.setLocation(x, y);
		button.setSize(60, 40);
		button.setText(String.valueOf(aantalStenen));
		button.setVisible(true);
		button.setEnabled(true);
		if (!isKalaha) {
			button.addActionListener(this);
		}
		button.setBackground(Color.ORANGE);
		button.setFocusable(false);
		button.setFont(new Font("arial", Font.BOLD, 16));
		add(button);

		return button;
	}
}
