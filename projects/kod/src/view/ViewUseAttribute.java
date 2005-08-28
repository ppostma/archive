/* $Id: ViewUseAttribute.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * ViewUseAttribute.java
 *
 * Use attributes class for the view.
 * (this is NOT equal to the Attributes class in the model!!)
 */

package src.view;

public class ViewUseAttribute
{
	public char type;
	public boolean active;

	public ViewUseAttribute(char type)
	{
		this.type = type;
		this.active = false;
	}
}
