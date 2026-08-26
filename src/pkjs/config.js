module.exports = [
  {
    "type": "heading",
    "defaultValue": "Cadence Pavé"
  },
  {
    "type": "text",
    "defaultValue": "The day as 24 blocks. Pick a livery and up to two complications."
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Livery" },
      {
        "type": "select",
        "messageKey": "LIVERY",
        "defaultValue": "0",
        "label": "Kit",
        "options": [
          { "label": "Zwift",                "value": "0" },
          { "label": "Surly",                "value": "1" },
          { "label": "EF Education",         "value": "2" },
          { "label": "Visma | Lease a Bike", "value": "3" },
          { "label": "Ineos '26",            "value": "4" },
          { "label": "La Vie Claire",        "value": "5" },
          { "label": "Mapei",                "value": "6" }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Complications" },
      {
        "type": "text",
        "defaultValue": "One fills the band; two split it. Sunrise and sunset are already drawn into the day cells — the complication adds the exact times."
      },
      {
        "type": "select",
        "messageKey": "COMP1",
        "defaultValue": "3",
        "label": "Left",
        "options": [
          { "label": "None",              "value": "0" },
          { "label": "Second time zone",  "value": "1" },
          { "label": "Moon phase",        "value": "2" },
          { "label": "Sunrise / sunset",  "value": "3" },
          { "label": "Weather",           "value": "4" },
          { "label": "Date",              "value": "5" }
        ]
      },
      {
        "type": "select",
        "messageKey": "COMP2",
        "defaultValue": "4",
        "label": "Right",
        "options": [
          { "label": "None",              "value": "0" },
          { "label": "Second time zone",  "value": "1" },
          { "label": "Moon phase",        "value": "2" },
          { "label": "Sunrise / sunset",  "value": "3" },
          { "label": "Weather",           "value": "4" },
          { "label": "Date",              "value": "5" }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Weather" },
      {
        "type": "radiogroup",
        "messageKey": "UNITS",
        "defaultValue": "0",
        "label": "Units",
        "options": [
          { "label": "Fahrenheit", "value": "0" },
          { "label": "Celsius",    "value": "1" }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Second time zone" },
      {
        "type": "input",
        "messageKey": "TZ_LABEL",
        "defaultValue": "",
        "label": "Label",
        "attributes": { "maxlength": "7", "placeholder": "defaults to the city" }
      },
      {
        "type": "select",
        "messageKey": "TZ_ZONE",
        "defaultValue": "Europe/Paris",
        "label": "Zone",
        "options": [
          { "label": "Honolulu", "value": "Pacific/Honolulu" },
          { "label": "Anchorage", "value": "America/Anchorage" },
          { "label": "Los Angeles", "value": "America/Los_Angeles" },
          { "label": "Vancouver", "value": "America/Vancouver" },
          { "label": "Phoenix", "value": "America/Phoenix" },
          { "label": "Denver", "value": "America/Denver" },
          { "label": "Chicago", "value": "America/Chicago" },
          { "label": "Mexico City", "value": "America/Mexico_City" },
          { "label": "New York", "value": "America/New_York" },
          { "label": "Toronto", "value": "America/Toronto" },
          { "label": "Bogotá", "value": "America/Bogota" },
          { "label": "Halifax", "value": "America/Halifax" },
          { "label": "St John's", "value": "America/St_Johns" },
          { "label": "São Paulo", "value": "America/Sao_Paulo" },
          { "label": "UTC", "value": "UTC" },
          { "label": "Reykjavík", "value": "Atlantic/Reykjavik" },
          { "label": "London", "value": "Europe/London" },
          { "label": "Dublin", "value": "Europe/Dublin" },
          { "label": "Lisbon", "value": "Europe/Lisbon" },
          { "label": "Paris", "value": "Europe/Paris" },
          { "label": "Brussels", "value": "Europe/Brussels" },
          { "label": "Amsterdam", "value": "Europe/Amsterdam" },
          { "label": "Madrid", "value": "Europe/Madrid" },
          { "label": "Rome", "value": "Europe/Rome" },
          { "label": "Berlin", "value": "Europe/Berlin" },
          { "label": "Zürich", "value": "Europe/Zurich" },
          { "label": "Oslo", "value": "Europe/Oslo" },
          { "label": "Athens", "value": "Europe/Athens" },
          { "label": "Helsinki", "value": "Europe/Helsinki" },
          { "label": "Cairo", "value": "Africa/Cairo" },
          { "label": "Johannesburg", "value": "Africa/Johannesburg" },
          { "label": "Moscow", "value": "Europe/Moscow" },
          { "label": "Nairobi", "value": "Africa/Nairobi" },
          { "label": "Dubai", "value": "Asia/Dubai" },
          { "label": "Karachi", "value": "Asia/Karachi" },
          { "label": "Kolkata", "value": "Asia/Kolkata" },
          { "label": "Kathmandu", "value": "Asia/Kathmandu" },
          { "label": "Dhaka", "value": "Asia/Dhaka" },
          { "label": "Bangkok", "value": "Asia/Bangkok" },
          { "label": "Singapore", "value": "Asia/Singapore" },
          { "label": "Hong Kong", "value": "Asia/Hong_Kong" },
          { "label": "Shanghai", "value": "Asia/Shanghai" },
          { "label": "Perth", "value": "Australia/Perth" },
          { "label": "Tokyo", "value": "Asia/Tokyo" },
          { "label": "Seoul", "value": "Asia/Seoul" },
          { "label": "Adelaide", "value": "Australia/Adelaide" },
          { "label": "Brisbane", "value": "Australia/Brisbane" },
          { "label": "Sydney", "value": "Australia/Sydney" },
          { "label": "Auckland", "value": "Pacific/Auckland" }
        ]
      },
      {
        "type": "text",
        "defaultValue": "Real zones, not fixed offsets — daylight saving is handled for you, and the offset is rechecked every half hour. Leave the label blank to use the city name."
      }
    ]
  },
  { "type": "submit", "defaultValue": "Save" }
];
