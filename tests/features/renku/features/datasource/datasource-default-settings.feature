Feature: Default data source
  As an advanced user
  I can choose the default data source
  In order to quickly store notes

  Scenario: Have a default data source for notes
    Given I display the flat data source list
    When the setting key defaultNoteCollection changes to 6
    Then the default data source is TestData/Private Notes

  Scenario: Change the default data source for notes
    Given I display the flat data source list
    And the setting key defaultNoteCollection changes to 42
    When the user changes the default data source to TestData/Private Notes
    Then the setting key defaultNoteCollection is 6
