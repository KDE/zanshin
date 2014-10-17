Feature: Context content
  As someone collecting tasks
  I can display a context
  In order to see the tasks associated to it

  @wip
  Scenario: Context tasks appear in the corresponding page
    Given I display the "Contexts / Errands" page
    And I look at the central list
    When I list the items
    Then the list is:
       | display                              |
       | Buy kiwis                            |
