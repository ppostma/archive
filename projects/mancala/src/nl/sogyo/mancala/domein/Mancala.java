package nl.sogyo.mancala.domein;

import nl.sogyo.mancala.domein.interfaces.IMancala;
import nl.sogyo.mancala.domein.interfaces.ISpeler;
import nl.sogyo.mancala.domein.interfaces.ISpelerListener;

/**
 * @author Peter Postma
 */
public class Mancala implements IMancala
{
	private Speler spelerA;
	private Speler spelerB;

	public Mancala(String naamA, String naamB, int aantalKommetjes,
			int aantalStenen)
	{
		this.spelerA = new Speler(naamA);
		this.spelerB = new Speler(naamB);

		this.spelerA.setTegenstander(spelerB);
		this.spelerB.setTegenstander(spelerA);

		Kom komA = new Kom(this.spelerA, aantalKommetjes, aantalStenen);
		Kom komB = new Kom(this.spelerB, aantalKommetjes, aantalStenen);

		komA.linkAanTegenstander(komB);
		komB.linkAanTegenstander(komA);

		this.spelerA.setKom(komA);
		this.spelerB.setKom(komB);

		this.spelerA.setAanZet(true);
	}

	public void addListener(ISpelerListener listener)
	{
		this.spelerA.addListener(listener);
		this.spelerB.addListener(listener);
	}

	public void removeListener(ISpelerListener listener)
	{
		this.spelerA.removeListener(listener);
		this.spelerB.removeListener(listener);
	}

	public ISpeler getSpelerAanZet()
	{
		return this.spelerA.isAanZet() ? this.spelerA : this.spelerB;
	}

	public ISpeler getWinnaar()
	{
		int aantalStenenA = this.spelerA.getTotaalAantalStenen();
		int aantalStenenB = this.spelerB.getTotaalAantalStenen();

		ISpeler winnaar = null;

		if (aantalStenenA > aantalStenenB) {
			winnaar = this.spelerA;
		} else if (aantalStenenA < aantalStenenB) {
			winnaar = this.spelerB;
		}

		return winnaar;
	}

	public ISpeler getSpeler1()
	{
		return this.spelerA;
	}

	public ISpeler getSpeler2()
	{
		return this.spelerB;
	}
}
