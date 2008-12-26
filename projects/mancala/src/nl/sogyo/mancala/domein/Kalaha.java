package nl.sogyo.mancala.domein;

import java.util.ArrayList;
import java.util.List;

import nl.sogyo.mancala.domein.excepties.MancalaException;
import nl.sogyo.mancala.domein.interfaces.IKom;

/**
 * @author Peter Postma
 */
public class Kalaha extends Kom
{
	public Kalaha(Speler eigenaar)
	{
		super(eigenaar, 0, 0);

		this.aantalStenen = 0;
	}

	@Override
	public void speel() throws MancalaException
	{
		throw new MancalaException("Kalaha kan niet gespeeld worden.");
	}

	@Override
	public IKom getKalaha()
	{
		return this;
	}

	@Override
	public List<IKom> getKommetjes()
	{
		return new ArrayList<IKom>();
	}

	@Override
	public int getTotaalAantalStenen()
	{
		return this.getAantalStenen();
	}

	@Override
	public boolean kanSpelen()
	{
		return false;
	}

	@Override
	protected boolean vullen(Speler speler, int aantal)
	{
		if (!this.eigenaar.equals(speler)) {
			return this.getBuurKom().vullen(speler, aantal);
		}

		this.addAantalStenen(1);

		this.eigenaar.sendKomUpdateEvent(this, false);

		if (--aantal > 0) {
			return this.getBuurKom().vullen(speler, aantal);
		}

		return true;
	}

	@Override
	protected void stenenNaarKalaha(Speler speler, int aantalStenen)
	{
		if (this.eigenaar.equals(speler)) {
			this.addAantalStenen(aantalStenen);

			this.eigenaar.sendKomUpdateEvent(this, false);
		} else {
			super.stenenNaarKalaha(speler, aantalStenen);
		}
	}

	@Override
	protected Kom getOverbuurKom(int aantalStappen)
	{
		return this.getKomOpPositie(aantalStappen);
	}

	@Override
	protected void linkAanTegenstander(Kom tegenstanderKom)
	{
		this.setBuurKom(tegenstanderKom);
	}
}
