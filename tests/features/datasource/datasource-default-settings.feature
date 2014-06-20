Feature: Data sources defaults
  As an advanced user
  I can choose the default data sources
  In order to quickly store artifacts outside of projects

  Scenario: Have a default data source for tasks in the inbox
    Given I'm looking at the inbox view
    When the setting key defaultCollection is 7
    Then the default task data source is TestData/Calendar1/Calendar2
