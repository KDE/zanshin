Feature: Editing notes
  As an organized person
  I can edit a previously created note
  In order to improve it, logging more ideas

  Scenario: Editing a note text
    Given I display the "Inbox" page
    And there is an item named "A page of diary" in the central list
    When I open the item in the editor
    And I change the editor text to "More on my day"
    And I open the item in the editor again
    Then the editor shows "More on my day" as text
