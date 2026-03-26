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
 * The example of the menu:
 * [
 *  {title: "Group 1", items: [{title: "Home", link: "/home"}, {title: "About", link: "/about"}]},
 *  {title: "Group 2", items: [{title: "Support", link: "/support"}, {title: "Contact", link: "/contact"}]},
 *  {title: "Group 3", items: [{title: "Documentation", link: "/documentation"}, {title: "Reference", link: "/reference"}]}
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
            let foundLink = group.items[0].link;
            for (let item of group.items) {
                if (item.link === pathname) {
                    foundGroup = group.title;
                    foundLink = item.link;
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

    onGroupClick(title)
    {
        this.setState({selectedGroup: title});
        if (this.props.onChange) {
            let link = this.state.selectedLinks[title];
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
        if (!groupIsSelected) {
            return <div key={"accordion-" + group.title} className="AccordionGroup"
                        onClick={() => this.onGroupClick(group.title)}>{group.title}</div>;
        }

        let items = [];
        for (let item of group.items) {
            let itemIsSelected = item.link === this.state.selectedLinks[group.title];
            let itemClass = itemIsSelected ? "AccordionItemSelected" : "AccordionItem";
            items.push(
                <div key={item.title + "-item"} className={itemClass}>
                    <NavLink key={item.title + "-navlink"} to={item.link}
                             onClick={() => this.onItemClick(item.link)}>{item.title}</NavLink>
                </div>);
        }
        return <div key={"accordion-group-" + group.title}>
            <div key={"accordion-" + group.title} className="AccordionGroup"
                 onClick={() => this.onGroupClick(group.title)}>{group.title}</div>
            {items}
        </div>;
    }

    render()
    {
        let groups = [];
        for (let group of this.props.menu) {
            groups.push(this.renderGroup(group, group.title === this.state.selectedGroup));
        }
        return <div key={"accordion"}>
            {groups}
        </div>;
    }
}
