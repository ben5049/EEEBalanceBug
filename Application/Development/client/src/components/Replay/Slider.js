import React, { useState } from 'react';

const Slider = () => {
  const [SliderValue, setSliderValue] = useState(0);

  const handleSliderChange = (event) => {
    setSliderValue(event.target.value);
  };

  return (
    <div>
      <div className="slidecontainer">
        <input
          type="range"
          min="1"
          max="100" // TODO: Change to number of entries in map table
          value={SliderValue}
          className="slider"
          id="myRange"
          onChange={handleSliderChange}
        />
        <p>Value: <span id="demo">{SliderValue}</span></p>
      </div>
    </div>
  );
};

export default Slider;
