import React, { useState } from 'react';

const Slider = () => {
  const [value, setValue] = useState(50);

  const handleSliderChange = (event) => {
    setValue(event.target.value);
  };

  return (
    <div>
      <div className="slidecontainer">
        <input
          type="range"
          min="1"
          max="100" // TODO: Change to number of entries in map table
          value={value}
          className="slider"
          id="myRange"
          onChange={handleSliderChange}
        />
        <p>Value: <span id="demo">{value}</span></p>
      </div>
    </div>
  );
};

export default Slider;
