import React from "react";
import "../css/Documentation.css";

export default class ComboBox extends React.Component
{
    render()
    {
        let items = [];
        for (let item of this.props.items)
        {
            items.push(<option key={this.props.name + "-" + item.value} value={item.value}>{item.text}</option>);
        }
        return <select style={{padding: 4, borderRadius: 4}} onChange={(e) => this.props.onChange(e.target.value)}>
            {items}
        </select>;
    }
}