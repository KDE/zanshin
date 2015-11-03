Feature: Default data source
  As an advanced user
  I can choose the default data source
  In order to quickly store tasks

  Scenario: Have a default data source for tasks in the inbox
    Given I display the flat data source list
    When the setting key defaultCollection changes to 7
    Then the default data source is TestData/Calendar1/Calendar2

  Scenario: Change the default data source for tasks in the inbox
    Given I display the flat data source list
    And the setting key defaultCollection changes to 42
    When the user changes the default data source to TestData/Calendar1/Calendar2
    Then the setting key defaultCollection is 7

