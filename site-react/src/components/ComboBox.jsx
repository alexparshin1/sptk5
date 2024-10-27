import React from "react";
import "../css/Documentation.css";

export default class ComboBox extends React.Component
{
    state = {
        fileName: ""
    };

    constructor(props)
    {
        super();
        this.name = props.name;
        this.items = props.items;
        this.onChange = props.onChange;
    }

    render()
    {
        let items = [];
        for (let item of this.items)
        {
            items.push(<option key={this.name + "-" + item.value} value={item.value}>{item.text}</option>);
        }
        return <select style={{padding: 4, borderRadius: 4}} onChange={(e) => this.onChange(e.target.value)}>
            {items}
        </select>;
    }
}
