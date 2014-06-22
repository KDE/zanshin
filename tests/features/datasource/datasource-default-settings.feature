Feature: Data sources defaults
  As an advanced user
  I can choose the default data sources
  In order to quickly store artifacts outside of projects

  Scenario: Have a default data source for tasks in the inbox
    Given I'm looking at the inbox view
    When the setting key defaultCollection changes to 7
    Then the default task data source is TestData/Calendar1/Calendar2

  Scenario: Change the default data source for tasks in the inbox
    Given I'm looking at the inbox view
    And the setting key defaultCollection changes to 42
    When the user changes the default task data source to TestData/Calendar1/Calendar2
    Then the setting key defaultCollection is 7

  Scenario: Have a default data source for notes in the inbox
    Given I'm looking at the inbox view
    When the setting key defaultNoteCollection changes to 6
    Then the default note data source is TestData/Private Notes

  Scenario: Change the default data source for notes in the inbox
    Given I'm looking at the inbox view
    And the setting key defaultNoteCollection changes to 42
    When the user changes the default note data source to TestData/Private Notes
    Then the setting key defaultNoteCollection is 6
