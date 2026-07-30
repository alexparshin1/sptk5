import React from "react";
import {NavLink} from "react-router-dom";
import "./Accordion.css";

/**
 * @brief Accordion component
 * Props:
 * - menu: list of menu items
 * CSS files:
 * - Accordion.css
 * Callback:
 * - onChange(link) when a menu item is selected
 *
 * The menu is a list of groups. Each group contains a list of items.
 * The first group and item are selected by default.
 *
 * An item may itself hold a nested list of sub-items (a sub-menu). Such an
 * item has no link of its own; it expands to show its sub-items when the
 * currently selected link belongs to it.
 *
 * The example of the menu:
 * [
 *  {title: "Group 1", items: [{title: "Home", link: "/home"}, {title: "About", link: "/about"}]},
 *  {title: "Group 2", items: [{title: "Support", link: "/support"}, {title: "Contact", link: "/contact"}]},
 *  {title: "Group 3", items: [
 *      {title: "Documentation", link: "/documentation"},
 *      {title: "Tests", items: [{title: "A", link: "/a"}, {title: "B", link: "/b"}]}
 *  ]}
 * ]
 */
export default class Accordion extends React.Component
{
    constructor(props)
    {
        super(props);
        const { menu } = props;
        const pathname = window.location.pathname;

        let foundGroup = menu[0].title;
        let selectedLinks = {};

        for (let group of menu) {
            let foundLink = Accordion.firstLink(group);
            for (let item of group.items) {
                const links = item.items ? item.items.map(sub => sub.link) : [item.link];
                if (links.includes(pathname)) {
                    foundGroup = group.title;
                    foundLink = pathname;
                    break;
                }
            }
            selectedLinks[group.title] = foundLink;
        }

        this.state = {
            selectedGroup: foundGroup,
            selectedLinks: selectedLinks
        };
    }

    // First navigable link of a group, descending into a nested item if needed
    static firstLink(group)
    {
        const first = group.items[0];
        return first.items ? first.items[0].link : first.link;
    }

    onGroupClick(title, link)
    {
        this.setState({selectedGroup: title});
        if (this.props.onChange) {
            this.props.onChange(link);
        }
    }

    onItemClick(link)
    {
        this.setState(prevState => {
            const newSelectedLinks = {
                ...prevState.selectedLinks,
                [prevState.selectedGroup]: link
            };
            return { selectedLinks: newSelectedLinks };
        });

        if (this.props.onChange) {
            this.props.onChange(link);
        }
    }

    renderGroup(group, groupIsSelected)
    {
        // Groups may appear after mount (e.g. Administration after login),
        // so fall back to the first link when the group isn't in selectedLinks yet
        const groupLink = this.state.selectedLinks[group.title] ?? Accordion.firstLink(group);

        if (!groupIsSelected) {
            return <NavLink key={"accordion-" + group.title} className="AccordionGroup"
                            to={groupLink}
                            onClick={() => this.onGroupClick(group.title, groupLink)}>{group.title}</NavLink>;
        }

        let items = [];
        for (let item of group.items) {
            items.push(...this.renderItem(item, groupLink));
        }
        return <div key={"accordion-group-" + group.title}>
            <NavLink key={"accordion-" + group.title} className="AccordionGroup AccordionGroupSelected"
                     to={groupLink}
                     onClick={() => this.onGroupClick(group.title, groupLink)}>{group.title}</NavLink>
            {items}
        </div>;
    }

    // Renders a single group item. A leaf item is a link; a nested item is a
    // sub-menu header that expands to its sub-items when the selected link
    // belongs to it. Returns an array of nodes.
    renderItem(item, groupLink)
    {
        if (!item.items) {
            let itemClass = item.link === groupLink ? "AccordionItemSelected" : "AccordionItem";
            return [
                <div key={item.title + "-item"} className={itemClass}>
                    <NavLink key={item.title + "-navlink"} to={item.link}
                             onClick={() => this.onItemClick(item.link)}>{item.title}</NavLink>
                </div>];
        }

        let isOpen = item.items.some(sub => sub.link === groupLink);
        // Clicking the header navigates to the first sub-item, which opens the sub-menu.
        let headerLink = item.items[0].link;
        let headerClass = isOpen ? "AccordionSubHeader AccordionSubHeaderSelected" : "AccordionSubHeader";
        let nodes = [
            <NavLink key={item.title + "-subheader"} className={headerClass}
                     to={headerLink}
                     onClick={() => this.onItemClick(headerLink)}>{item.title}</NavLink>];
        if (isOpen) {
            for (let sub of item.items) {
                let subClass = sub.link === groupLink ? "AccordionSubItemSelected" : "AccordionSubItem";
                nodes.push(
                    <div key={sub.title + "-subitem"} className={subClass}>
                        <NavLink key={sub.title + "-navlink"} to={sub.link}
                                 onClick={() => this.onItemClick(sub.link)}>{sub.title}</NavLink>
                    </div>);
            }
        }
        return nodes;
    }

    render()
    {
        let groups = [];
        for (let group of this.props.menu) {
            groups.push(this.renderGroup(group, group.title === this.state.selectedGroup));
        }
        return <div key={"accordion"} className="Accordion">
            {groups}
        </div>;
    }
}
