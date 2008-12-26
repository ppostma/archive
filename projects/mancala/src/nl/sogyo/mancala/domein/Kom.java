package nl.sogyo.mancala.domein;

import java.util.ArrayList;
import java.util.List;

import nl.sogyo.mancala.domein.excepties.MancalaException;
import nl.sogyo.mancala.domein.interfaces.IKom;
import nl.sogyo.mancala.domein.interfaces.ISpeler;

/**
 * @author Peter Postma
 */
public class Kom implements IKom
{
	protected int aantalStenen;
	protected Speler eigenaar;
	protected Kom buurKom;

	public Kom(Speler eigenaar, int aantalKommetjes, int aantalStenen)
	{
		this.aantalStenen = aantalStenen;
		this.eigenaar = eigenaar;
		
		if (aantalKommetjes > 1) {
			this.buurKom = new Kom(eigenaar, --aantalKommetjes, aantalStenen);
		} else if (aantalKommetjes == 1) {
			this.buurKom = new Kalaha(eigenaar);
		}
	}

	public void speel() throws MancalaException
	{
		if (!this.eigenaar.isAanZet()) {
			throw new MancalaException("Kan kom niet kiezen, speler is niet aan zet.");
		}
		if (this.getAantalStenen() == 0) {
			throw new MancalaException("Deze kom bevat geen stenen.");
		}

		int aantal = this.getAantalStenen();
		this.setAantalStenen(0);

		this.eigenaar.sendKomUpdateEvent(this, false);

		boolean nogEenKeer = this.getBuurKom().vullen(this.eigenaar, aantal);

		this.eigenaar.setAanZet(nogEenKeer);
	}

	public Kom getKomOpPositie(int positie)
	{
		if (positie-- > 0) {
			return this.getBuurKom().getKomOpPositie(positie);
		}

		return this;
	}

	public IKom getKalaha()
	{
		return this.getBuurKom().getKalaha();
	}

	public List<IKom> getKommetjes()
	{
		List<IKom> kommetjes = new ArrayList<IKom>();
		kommetjes.add(this);
		kommetjes.addAll(this.getBuurKom().getKommetjes());

		return kommetjes;
	}

	public int getAantalStenen()
	{
		return this.aantalStenen;
	}

	public int getTotaalAantalStenen()
	{
		return this.getAantalStenen() + this.getBuurKom().getTotaalAantalStenen();
	}

	public boolean kanSpelen()
	{
		if (this.getAantalStenen() > 0) {
			return true;
		}

		return this.getBuurKom().kanSpelen();
	}

	public ISpeler getEigenaar()
	{
		return this.eigenaar;
	}

	protected boolean vullen(Speler speler, int aantal)
	{
		this.addAantalStenen(1);

		this.eigenaar.sendKomUpdateEvent(this, false);

		if (--aantal > 0) {
			return this.getBuurKom().vullen(speler, aantal);
		}

		if (isEenSlag(speler)) {
			this.maakSlag(speler);
			this.getOverbuurKom(0).maakSlag(speler);
		}

		return false;
	}

	protected void maakSlag(Speler speler)
	{
		int aantal = this.getAantalStenen();
		this.setAantalStenen(0);

		this.eigenaar.sendKomUpdateEvent(this, true);

		this.stenenNaarKalaha(speler, aantal);
	}

	protected void stenenNaarKalaha(Speler speler, int aantalStenen)
	{
		this.getBuurKom().stenenNaarKalaha(speler, aantalStenen);
	}

	protected Kom getBuurKom()
	{
		return this.buurKom;
	}

	protected Kom getOverbuurKom(int aantalStappen)
	{
		return this.getBuurKom().getOverbuurKom(++aantalStappen);
	}

	protected void setBuurKom(Kom kom)
	{
		this.buurKom = kom;		
	}

	protected void linkAanTegenstander(Kom tegenstanderKom)
	{
		this.getBuurKom().linkAanTegenstander(tegenstanderKom);
	}

	protected void setAantalStenen(int aantalStenen)
	{
		this.aantalStenen = aantalStenen;
	}

	protected void addAantalStenen(int aantalStenen)
	{
		this.aantalStenen += aantalStenen;
	}

	private boolean isEenSlag(ISpeler speler)
	{
		return this.getAantalStenen() == 1 && this.eigenaar.equals(speler);
	}
}
