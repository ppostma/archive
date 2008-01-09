package nl.pointless.sudoku;

/**
 * @author Peter Postma
 */
public class Box
{
	private int x;
	private int y;
	private int value;
	private boolean mutable;

	public Box(int x, int y, int value)
	{
		this.x = x;
		this.y = y;
		this.value = value;
		this.mutable = true;
	}

	public int getX()
	{
		return this.x;
	}

	public int getY()
	{
		return this.y;
	}

	public int getValue()
	{
		return this.value;
	}

	public void setValue(int value)
	{
		if (isMutable())
			this.value = value;
	}

	public void removeValue()
	{
		this.value = 0;
	}

	public boolean isMutable()
	{
		return this.mutable;
	}

	public void setMutable(boolean mutable)
	{
		this.mutable = mutable;
	}
}
