package nl.pointless.webmail.web.component;

import org.apache.wicket.markup.html.image.ContextImage;
import org.apache.wicket.markup.html.link.AbstractLink;
import org.apache.wicket.markup.html.navigation.paging.IPageable;
import org.apache.wicket.markup.html.navigation.paging.IPagingLabelProvider;
import org.apache.wicket.markup.html.navigation.paging.PagingNavigation;
import org.apache.wicket.markup.html.navigation.paging.PagingNavigator;

/**
 * PagingNavigator used by {@link MessageListDataTable}.
 * 
 * @author Peter Postma
 */
public class MessageListPagingNavigator extends PagingNavigator {

	private static final long serialVersionUID = 1L;

	/**
	 * Constructor.
	 * 
	 * @param id Component Id.
	 * @param pageable The pageable component the page links are referring to.
	 */
	public MessageListPagingNavigator(String id, IPageable pageable) {
		super(id, pageable);
	}

	@Override
	protected AbstractLink newPagingNavigationLink(String id,
			IPageable pageable, int pageNumber) {
		AbstractLink newPagingNavigationLink = super.newPagingNavigationLink(
				id, pageable, pageNumber);

		if (id.equals("first")) {
			ContextImage image = new ContextImage("imageFirst",
					"images/arrow_first.png");
			newPagingNavigationLink.add(image);
		} else {
			ContextImage image = new ContextImage("imageLast",
					"images/arrow_last.png");
			newPagingNavigationLink.add(image);
		}

		return newPagingNavigationLink;
	}

	@Override
	protected AbstractLink newPagingNavigationIncrementLink(String id,
			IPageable pageable, int increment) {
		AbstractLink newPagingNavigationIncrementLink = super
				.newPagingNavigationIncrementLink(id, pageable, increment);

		if (id.equals("prev")) {
			ContextImage image = new ContextImage("imagePrev",
					"images/arrow_left.png");
			newPagingNavigationIncrementLink.add(image);
		} else {
			ContextImage image = new ContextImage("imageNext",
					"images/arrow_right.png");
			newPagingNavigationIncrementLink.add(image);
		}

		return newPagingNavigationIncrementLink;
	}

	@Override
	protected PagingNavigation newNavigation(final String id,
			final IPageable pageable, final IPagingLabelProvider labelProvider) {
		PagingNavigation pagingNavigation = new PagingNavigation(
				id /* "navigation" */, pageable, labelProvider);
		pagingNavigation.setSeparator("&thinsp;");
		return pagingNavigation;
	}

	@Override
	protected void onConfigure() {
		super.onConfigure();

		setVisible(getPageable().getPageCount() > 1);
	}
}
