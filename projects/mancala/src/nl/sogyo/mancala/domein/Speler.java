package nl.sogyo.mancala.domein;

import java.util.ArrayList;
import java.util.List;

import nl.sogyo.mancala.domein.interfaces.IKom;
import nl.sogyo.mancala.domein.interfaces.ISpeler;
import nl.sogyo.mancala.domein.interfaces.ISpelerListener;

/**
 * @author Peter Postma
 */
public class Speler implements ISpeler
{
	private List<ISpelerListener> listeners;
	private IKom eersteKom;
	private boolean isAanZet;
	private String naam;
	private Speler tegenstander;

	public Speler(String naam)
	{
		this.listeners = new ArrayList<ISpelerListener>();
		this.naam = naam;
		this.isAanZet = false;
	}

	public void addListener(ISpelerListener listener)
	{
		this.listeners.add(listener);
	}

	public void removeListener(ISpelerListener listener)
	{
		this.listeners.add(listener);
	}
	
	public String getNaam()
	{
		return this.naam;
	}

	public IKom getKomOpPositie(int positie)
	{
		return this.eersteKom.getKomOpPositie(positie);
	}

	public IKom getKalaha()
	{
		return this.eersteKom.getKalaha();
	}
	
	public List<IKom> getKommetjes()
	{
		return this.eersteKom.getKommetjes();
	}

	public int getTotaalAantalStenen()
	{
		return this.eersteKom.getTotaalAantalStenen();
	}

	public boolean kanZetDoen()
	{
		return this.eersteKom.kanSpelen();
	}

	void setKom(IKom kom)
	{
		this.eersteKom = kom;
	}

	void setTegenstander(Speler tegenstander)
	{
		this.tegenstander = tegenstander;
	}

	boolean isAanZet()
	{
		return this.isAanZet;
	}

	void setAanZet(boolean isAanZet)
	{
		this.isAanZet = isAanZet;

		if (this.tegenstander.isAanZet() == isAanZet) {
			this.tegenstander.setAanZet(!isAanZet);
		}

		if (this.isAanZet) {
			this.sendSpelerWisselBeurtEvent(this);

			if (!this.kanZetDoen()) {
				this.sendSpelAfgelopenEvent();
			}
		}
	}

	void sendKomUpdateEvent(IKom kom, boolean isSlag)
	{
		for (ISpelerListener listener : this.listeners) {
			listener.komUpdateEvent(kom, isSlag);
		}
	}

	void sendSpelAfgelopenEvent()
	{
		for (ISpelerListener listener : this.listeners) {
			listener.spelAfgelopenEvent();
		}
	}

	void sendSpelerWisselBeurtEvent(ISpeler speler)
	{
		for (ISpelerListener listener : this.listeners) {
			listener.spelerWisselBeurtEvent(speler);
		}
	}
}
